/* (c) 2016, 2017 DECENT Services. For details refers to LICENSE.txt */
/*
 * Copyright (c) 2015 Cryptonomex, Inc., and contributors.
 *
 * The MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <graphene/app/application.hpp>

#include <graphene/miner/miner.hpp>
#include <graphene/seeding/seeding.hpp>
#include <graphene/account_history/account_history_plugin.hpp>
#include <graphene/transaction_history/transaction_history_plugin.hpp>
#include <graphene/messaging/messaging.hpp>
#include <graphene/utilities/dirhelper.hpp>
#include <graphene/utilities/git_revision.hpp>

#include <fc/exception/exception.hpp>
#include <fc/thread/thread.hpp>
#include <fc/interprocess/signals.hpp>
#include <fc/log/console_appender.hpp>
#include <fc/log/file_appender.hpp>
#include <fc/log/logger.hpp>
#include <fc/log/logger_config.hpp>

#include <boost/version.hpp>
#include <openssl/opensslv.h>
#include <cryptopp/config.h>

#include <decent/config/decent_log_config.hpp>
#include <decent/monitoring/monitoring_fc.hpp>

#include <iostream>
#include <fstream>

#ifdef WIN32
#include <signal.h> 
#include <windows.h>
#else
# include <csignal>
#endif

using namespace graphene;
namespace bpo = boost::program_options;
         


#ifdef _MSC_VER
// Stopping from GUI
static fc::promise<int>::ptr s_exit_promise;
BOOL s_bStop = FALSE;
HANDLE s_hStopProcess = NULL;

DWORD WINAPI StopFromGUIThreadProc(void* params)
{
   while (s_bStop == FALSE)
   {
      if (WaitForSingleObject(s_hStopProcess, 0) == 0)
         s_bStop = TRUE;
      Sleep(50);
   }
   CloseHandle(s_hStopProcess);
   elog("Caught stop from GUI attempting to exit cleanly");
   s_exit_promise->set_value(SIGTERM);
   return 0;
}
BOOL WINAPI HandlerRoutine(_In_ DWORD dwCtrlType) {
   switch (dwCtrlType)
   {
   case CTRL_C_EVENT:
      elog("Caught stop by Ctrl+C to exit cleanly");
      s_exit_promise->set_value(SIGTERM);
      return TRUE;
   case CTRL_BREAK_EVENT:
      elog("Caught stop by Ctrl+break to exit cleanly");
      s_exit_promise->set_value(SIGTERM);
      return TRUE;
   case CTRL_CLOSE_EVENT:
      elog("Caught stop by closing console window to exit cleanly");
      s_exit_promise->set_value(SIGTERM);
      return TRUE;
   case CTRL_LOGOFF_EVENT:
      elog("Caught stop by logoff event to exit cleanly");
      s_exit_promise->set_value(SIGTERM);
      return TRUE;
   case CTRL_SHUTDOWN_EVENT:
      elog("Caught stop by shutdown event to exit cleanly");
      s_exit_promise->set_value(SIGTERM);
      return TRUE;
   default:
      return FALSE;
   }
}
#endif

#if defined(__linux__) || defined(__APPLE__)

int start_as_daemon()
{
   pid_t pid, sid;

   /* Fork off the parent process */
   pid = fork();
   if (pid < 0) {
       return -1;
   }
   /* If we got a good PID, then
     we can exit the parent process. */
   if (pid > 0) {
       return 1;
   }

   /* Change the file mode mask */
   umask(0);

   /* Open any logs here */

   /* Create a new SID for the child process */
   sid = setsid();
   if (sid < 0) {
       return -1;
   }

   /* Change the current working directory */
   if ((chdir("/")) < 0) {
      return -1;
   }

   /* Close out the standard file descriptors */
   close(STDIN_FILENO);
   close(STDOUT_FILENO);
   close(STDERR_FILENO);

   return 0;
}
#else

int start_as_daemon()
{
   //NOTE: this OS is not supported yet...
   return -1;
}

#endif


int main(int argc, char** argv) {
#ifdef _MSC_VER
   SetConsoleCtrlHandler(HandlerRoutine, TRUE);
#endif

   bpo::options_description app_options("DECENT Daemon");
   bpo::options_description cfg_options("DECENT Daemon");
   bpo::variables_map options;

   using decent_plugins = graphene::app::plugin_set<
      miner_plugin::miner_plugin,
      account_history::account_history_plugin,
      decent::seeding::seeding_plugin,
      decent::messaging::messaging_plugin,
      transaction_history::transaction_history_plugin
   >;

   try
   {
      bpo::options_description cli, cfg;
      app::application::set_program_options(cli, cfg);
      decent_plugins::set_program_options(cli, cfg);
      cli.add_options()
         ("daemon", "Run DECENT as daemon.")
      ;

      app_options.add(cli);
      cfg_options.add(cfg);
      bpo::store(bpo::parse_command_line(argc, argv, app_options), options);
   }
   catch (const boost::program_options::error& e)
   {
     std::cerr << "Error parsing command line: " << e.what() << "\n";
     return EXIT_FAILURE;
   }

   if( options.count("help") )
   {
      std::cout << app_options << std::endl;
      return EXIT_SUCCESS;
   }
   else if( options.count("version") )
   {
      unsigned int boost_major_version = BOOST_VERSION / 100000;
      unsigned int boost_minor_version = BOOST_VERSION / 100 - boost_major_version * 1000;
      std::string boost_version_text = std::to_string(boost_major_version) + "." + std::to_string(boost_minor_version) + "." + std::to_string(BOOST_VERSION % 100);
      std::string openssl_version_text = std::string(OPENSSL_VERSION_TEXT);
      openssl_version_text = openssl_version_text.substr(0, openssl_version_text.length() - 11);
      std::string cryptopp_version_text = std::to_string(CRYPTOPP_VERSION / 100) + "." + std::to_string(CRYPTOPP_VERSION % 100);

      std::cout << "DECENT Daemon " << graphene::utilities::git_version();
#ifndef NDEBUG
      std::cout << " (debug)";
#endif /* NDEBUG */
      std::cout << "\nBoost " << boost_version_text << "\n" << openssl_version_text << "\nCryptopp " << cryptopp_version_text << std::endl;
      return EXIT_SUCCESS;
   }

   app::application* node = new app::application();
   fc::oexception unhandled_exception;
   try {
      decent_plugins::types plugins = decent_plugins::create(*node);

      bool run_as_daemon = false;
      fc::path logs_dir, data_dir, temp_dir, config_filename;
      auto& path_finder = utilities::decent_path_finder::instance();
      if (options.count("daemon")) {
         int ret = start_as_daemon();
         if (ret < 0) {
            std::cerr << "Error running as daemon.\n";
            return 1;
         }
         else if (ret == 1) {
            return 0;
         }

         //continue as daemon...
         run_as_daemon = true;

         //default path settings for daemon
         config_filename = "/etc/decentd";
         logs_dir = "/var/log/decentd/";
         data_dir = "/var/lib/decentd/";
         temp_dir = "/var/tmp/decentd/";

         path_finder.set_decent_logs_path(logs_dir);
         path_finder.set_decent_data_path(data_dir);
         path_finder.set_decent_temp_path(temp_dir);
      }
      else {
         data_dir   = path_finder.get_decent_data();
         logs_dir   = path_finder.get_decent_logs();
         temp_dir   = path_finder.get_decent_temp();

         if( options.count("data-dir") )
         {
            data_dir = options["data-dir"].as<boost::filesystem::path>();
            if( data_dir.is_relative() )
               data_dir = fc::current_path() / data_dir;
         }

         config_filename = data_dir / "config.ini";

         //NOTE: make it work as till now...
         logs_dir = fc::absolute(config_filename).parent_path();
      }

      if( fc::exists(config_filename) )
      {
         // get the basic options
         bpo::store(bpo::parse_config_file<char>(config_filename.preferred_string().c_str(), cfg_options, true), options);
         // try to get logging options from the config file.
         try
         {
            fc::optional<fc::logging_config> logging_config = decent::load_logging_config_from_ini_file(config_filename, logs_dir);
            if (logging_config) {
               if (!fc::configure_logging(*logging_config)) {
                  if (run_as_daemon) {
                     std::cerr << "Error configure logging!\n";
                  }
                  else {
                     wlog("Error configure logging!");
                  }
                  return 1;
               }
            }
         }
         catch (const fc::exception&)
         {
            wlog("Error parsing logging config from config file ${cfg}, using default config", ("cfg", config_filename.preferred_string()));
         }
      }
      else 
      {
         //NOTE: We should not write a config when we run as daemon, but for now we leave it as is.

         ilog("Writing new config file at ${path}", ("path", config_filename));
         if( !fc::exists(data_dir) )
            fc::create_directories(data_dir);

         std::ofstream out_cfg(config_filename.preferred_string());
         for( const boost::shared_ptr<bpo::option_description> od : cfg_options.options() )
         {
            if( !od->description().empty() )
               out_cfg << "# " << od->description() << "\n";
            boost::any store;
            if( !od->semantic()->apply_default(store) )
               out_cfg << "# " << od->long_name() << " = \n";
            else {
               auto example = od->format_parameter();
               if( example.empty() )
                  // This is a boolean switch
                  out_cfg << od->long_name() << " = " << "false\n";
               else {
                  // The string is formatted "arg (=<interesting part>)"
                  example.erase(0, 6);
                  example.erase(example.length()-1);
                  out_cfg << od->long_name() << " = " << example << "\n";
               }
            }
            out_cfg << "\n";
         }
         decent::write_default_logging_config_to_stream(out_cfg, run_as_daemon);
         out_cfg.close(); 
         // read the default logging config we just wrote out to the file and start using it
         fc::optional<fc::logging_config> logging_config = decent::load_logging_config_from_ini_file(config_filename, logs_dir);
         if (logging_config)
            fc::configure_logging(*logging_config);
      }
      monitoring::set_data_dir(data_dir);
      monitoring::monitoring_counters_base::start_monitoring_thread();

      bpo::notify(options);
      node->initialize(data_dir, options);
      node->initialize_plugins( options );

      node->startup();
      node->startup_plugins();

      

      fc::promise<int>::ptr exit_promise = new fc::promise<int>("UNIX Signal Handler");
#ifdef _MSC_VER
      s_exit_promise = exit_promise;
      s_hStopProcess = OpenEventA(SYNCHRONIZE, FALSE, "A883EF36-8168-4E45-A08F-97EFFC5B6694");
      if (s_hStopProcess)
      {
         DWORD tid = 0;
         CreateThread(NULL, 0, StopFromGUIThreadProc, NULL, 0, &tid);
      }
#else
      fc::set_signal_handler([&exit_promise](int signal) {
         elog( "Caught SIGINT attempting to exit cleanly" );
         exit_promise->set_value(signal);
      }, SIGINT);

      fc::set_signal_handler([&exit_promise](int signal) {
         elog( "Caught SIGTERM attempting to exit cleanly" );
         exit_promise->set_value(signal);
      }, SIGTERM);

      fc::set_signal_handler([&exit_promise](int signal) {
           elog( "Caught SIGHUP attempting to exit cleanly" );
           exit_promise->set_value(signal);
      }, SIGHUP);
#endif
      ilog("Started miner node on a chain with ${h} blocks.", ("h", node->chain_database()->head_block_num()));
      ilog("Chain ID is ${id}", ("id", node->chain_database()->get_chain_id()) );

      auto seeding_plug = std::get<2>(plugins);
      if( !seeding_plug->my )
      {
         decent::seeding::seeding_promise = new fc::promise<decent::seeding::seeding_plugin_startup_options>("Seeding Promise");

         while( !decent::seeding::seeding_promise->ready() && !exit_promise->ready() ){
            fc::usleep(fc::microseconds(1000000));
         }

         if( decent::seeding::seeding_promise->ready() && !exit_promise->ready() ){
            seeding_plug->plugin_pre_startup(decent::seeding::seeding_promise->wait(fc::microseconds(1)));
            seeding_plug->plugin_startup();
         }
      }

      int signal = exit_promise->wait();
      ilog("Exiting from signal ${n}", ("n", signal));
      node->shutdown_plugins();
      monitoring::monitoring_counters_base::stop_monitoring_thread();
      node->shutdown();
      delete node;
#ifdef _MSC_VER
      s_bStop = TRUE;
#endif
      return 0;
   } catch( const fc::exception& e ) {
      // deleting the node can yield, so do this outside the exception handler
      unhandled_exception = e;
   }
#ifdef _MSC_VER
   s_bStop = TRUE;
#endif
   if (unhandled_exception)
   {
      elog("Exiting with error:\n${e}", ("e", unhandled_exception->to_detail_string()));
      monitoring::monitoring_counters_base::stop_monitoring_thread();
      node->shutdown();
      delete node;
      return 1;
   }
}






