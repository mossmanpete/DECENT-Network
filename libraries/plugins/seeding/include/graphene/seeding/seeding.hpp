/* (c) 2016, 2017 DECENT Services. For details refers to LICENSE.txt */
#pragma once

#include <graphene/app/plugin.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/content_object.hpp>

#include <graphene/chain/protocol/decent.hpp>
#include <graphene/db/object.hpp>
#include <graphene/db/generic_index.hpp>
#include <graphene/chain/protocol/types.hpp>
#include <graphene/app/seeding_utility.hpp>
#include <decent/package/package.hpp>

namespace decent { namespace seeding {

using namespace graphene::chain;

#ifndef SEEDING_PLUGIN_SPACE_ID
#define SEEDING_PLUGIN_SPACE_ID 123
#endif

enum seeding_object_type
{
   seeder_object_type = 0,
   seeding_object_type = 1
};

class seeding_plugin;

/**
 * @Class my_seeder_object - Defines seeders managed by this plugin. ATM only one seeder is supported.
 */
class my_seeder_object : public graphene::db::abstract_object< my_seeder_object >
{
public:
   static const uint8_t space_id = SEEDING_PLUGIN_SPACE_ID;
   static const uint8_t type_id  = seeder_object_type;

   account_id_type seeder;
   decent::encrypt::DIntegerString content_privKey;

   fc::ecc::private_key privKey;
   uint64_t free_space;
   string region_code;
   string price;
   string symbol;

};

/**
 * @class my_seeding_object - Defines content pieces seeded by this plugin.
 */
class my_seeding_object : public graphene::db::abstract_object< my_seeding_object >
{
public:
   static const uint8_t space_id = SEEDING_PLUGIN_SPACE_ID;
   static const uint8_t type_id  = seeding_object_type;

   string URI; //<Content address
   fc::ripemd160 _hash; //<Content hash
   fc::time_point_sec expiration; //<Content expiration
   fc::optional<decent::encrypt::CustodyData> cd; //<Content custody data

   account_id_type seeder; //<Seeder seeding this content managed by this plugin
   decent::encrypt::CiphertextString key; //<Decryption key part

   uint32_t space;
   bool downloaded = false;
   bool deleted = false;
   const content_object& get_content(database &db)const{
      const auto& cidx = db.get_index_type<content_index>().indices().get<graphene::chain::by_URI>();
      const auto& citr = cidx.find(URI);
      FC_ASSERT(citr!=cidx.end());
      return *citr;
   };
};

typedef graphene::chain::object_id< SEEDING_PLUGIN_SPACE_ID, seeding_object_type,  my_seeding_object>     my_seeding_id_type;
typedef graphene::chain::object_id< SEEDING_PLUGIN_SPACE_ID, seeder_object_type,  my_seeder_object>     my_seeder_id_type;

struct by_id;
struct by_URI;
struct by_hash;
struct by_seeder;

typedef multi_index_container<
      my_seeder_object,
      indexed_by<
            ordered_unique< tag<by_id>, member< object, object_id_type, &object::id >>,
            ordered_unique< tag< by_seeder >, member< my_seeder_object, account_id_type, &my_seeder_object::seeder> >
      >
>my_seeder_object_multi_index_type;

typedef generic_index< my_seeder_object, my_seeder_object_multi_index_type > my_seeder_index;

typedef multi_index_container<
      my_seeding_object,
      indexed_by<
            ordered_unique< tag<by_id>, member< object, object_id_type, &object::id >>,
            ordered_unique< tag< by_URI >, member< my_seeding_object, string, &my_seeding_object::URI> >,
            ordered_unique< tag< by_hash >, member< my_seeding_object, fc::ripemd160, &my_seeding_object::_hash> >
      >
>my_seeding_object_multi_index_type;

typedef generic_index< my_seeding_object, my_seeding_object_multi_index_type > my_seeding_index;


namespace detail {

class SeedingListener;



/**
 * @class seeding_plugin_impl This class implements the seeder functionality.
 * @inherits package_transfer_interface::transfer_listener Integrates with package manager through this interface.
 */
class seeding_plugin_impl /*: public package_transfer_interface::transfer_listener */{
public:
   seeding_plugin_impl(seeding_plugin &_plugin) : _self(_plugin) { }

   ~seeding_plugin_impl();

   /**
    * Get DB instance
    * @return DB instance
    */
   graphene::chain::database &database();
   /**
    * Generates proof of retrievability of a package
    * @param so_id ID of the my_seeding_object
    * @param downloaded_package Downloaded package object
    */
   //void generate_por( my_seeding_id_type so_id, graphene::package::package_object downloaded_package );

   /**
    * Generates proof of retrievability of a package
    * @param so_id ID of the my_seeding_object
    * @param downloaded_package Downloaded package object
    */
   void generate_pors();

   void generate_por_int(const my_seeding_object &so, decent::package::package_handle_t package_handle, fc::ecc::private_key privKey);
   void generate_por_int(const my_seeding_object &so, decent::package::package_handle_t package_handle);
   /**
    * Process new content, from content_object
    * @param co Content object
    */
   void handle_new_content(const content_object& co);

   /**
    * Delete data and database object related to a package. Called e.g. on package expiration
    * @param mso database object
    * @param package_handle package handle
    */
   void release_package(const my_seeding_object &mso, decent::package::package_handle_t package_handle);
   /**
    * Process new content, from operation. If the content is managed by local seeder, it is downloaded, and meta are stored in local db.
    * @param cs_op
    */
   void handle_new_content(const content_submit_operation& cs_op);
   /**
    * Handle newly submitted or resubmitted content. If it is content managed by one of our seeders, download it.
    * @param op_obj The operation wrapper carrying content submit operation
    */
   void handle_content_submit(const content_submit_operation &op);

   /**
    * Handle request to buy. If it is concerning one of content seeded by the plugin, provide decryption key parts in deliver key
    * @param op_obj The operation wrapper carrying content request to buy operation
    */
   void handle_request_to_buy(const request_to_buy_operation &op);

   /**
    * Called only after the highest known block has been applied. If it is request to buy or content submit, pass it to the corresponding handler
    * @param op_obj The operation wrapper
    * @param sync_mode
    */
   void handle_commited_operation(const operation_history_object &op_obj, bool sync_mode);

   /**
    * Restarts all downloads and seeding upon application start
    */
   void restore_state();

   /**
    * Resend all possibly missed keys
    */
   void resend_keys();


   /**
    * Generates and broadcasts RtP operation
    */
   void send_ready_to_publish();

   std::vector<SeedingListener> listeners;
   seeding_plugin& _self;
//   std::map<package_transfer_interface::transfer_id, my_seeding_id_type> active_downloads; //<List of active downloads for whose we are expecting on_download_finished callback to be called
   std::shared_ptr<fc::thread> service_thread; //The thread where the computation shall happen
   fc::thread* main_thread; //The main thread, used mainly for DB modifications

};

class SeedingListener : public decent::package::EventListenerInterface, public std::enable_shared_from_this<SeedingListener> {
private:
   string _url;
   decent::package::package_handle_t _pi;
   seeding_plugin_impl *_my;
   int failed=0;
public:
   SeedingListener(seeding_plugin_impl &impl, const my_seeding_object &mso,
                   const decent::package::package_handle_t pi) {
      _url = mso.URI;
      _pi = pi;
      _my = &impl;
   };

   ~SeedingListener() {};

   virtual void package_download_error(const std::string &);
   virtual void package_download_complete();
};

} //namespace detail





class seeding_plugin : public graphene::app::plugin
{
   public:
      seeding_plugin(graphene::app::application* app);
      ~seeding_plugin(){};

      static std::string plugin_name();

      /**
       * Extend program options with our option list
       * @param cli CLI parameters
       * @param cfg Config file parameters
       */
      static void plugin_set_program_options(
              boost::program_options::options_description& cli,
              boost::program_options::options_description& cfg);

      /**
       * Initialize plugin based on config parameters
       * @param options
       */
      void plugin_initialize(const boost::program_options::variables_map& options) override;
      /**
       * Pre-startup step of the seeding plugin
       * @param seeding_options
       */
      void plugin_pre_startup( const seeding_plugin_startup_options& seeding_options );
      /**
       * Start the plugin and begin work.
       */
      void plugin_startup() override;


      friend class detail::seeding_plugin_impl;
      std::unique_ptr<detail::seeding_plugin_impl> my;
};

}}

FC_REFLECT_DERIVED( decent::seeding::my_seeder_object, (graphene::db::object), (seeder)(content_privKey)(privKey)(free_space)(region_code)(price)(symbol) );
FC_REFLECT_DERIVED( decent::seeding::my_seeding_object, (graphene::db::object), (URI)(expiration)(cd)(seeder)(key)(space)(downloaded)(deleted)(_hash) );


