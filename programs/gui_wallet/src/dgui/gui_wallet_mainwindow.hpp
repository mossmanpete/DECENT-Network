/*
 *	File: gui_wallet_mainwindow.cpp
 *
 *	Created on: Nov 11, 2016
 *	Created by: Davit Kalantaryan (Email: davit.kalantaryan@desy.de)
 *
 *  This file implements ...
 *
 */
#ifndef MAINWINDOW_GUI_WALLET_H
#define MAINWINDOW_GUI_WALLET_H

#include <QMainWindow>
#include "gui_wallet_centralwidget.hpp"
#include <QAction>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "gui_wallet_connectdlg.hpp"
#include "text_display_dialog.hpp"
#include "richdialog.hpp"
#include "cliwalletdlg.hpp"
#include <unnamedsemaphorelite.hpp>
#include <stdarg.h>
#include <string>
#include <map>
#include "decent_wallet_ui_gui_contentdetailsgeneral.hpp"
#include "decent_wallet_ui_gui_contentdetailsbougth.hpp"

namespace gui_wallet
{

    class Mainwindow_gui_wallet : public QMainWindow
    {

        Q_OBJECT
    public:
        Mainwindow_gui_wallet();
        virtual ~Mainwindow_gui_wallet();   // virtual because may be this class will be
                                            // used by inheritanc
        void SetNewTaskQtMainWnd2(const std::string& a_inp_line, void* a_clbData);
        void SetNewTaskQtMainWnd3(const std::string& a_inp_line, void* a_clbData);

    protected:
        void CreateActions();
        void CreateMenues();

        void TaskDoneFuncGUI(void* clbkArg,int64_t err,const std::string& task,const std::string& result);
        void ManagementNewFuncGUI(void* clbkArg,int64_t err,const std::string& task,const std::string& result);

        void TaskDoneBrowseContentGUI(void* a_clbkArg,int64_t a_err,const std::string& a_task,const std::string& a_result);
        void TaskDoneTransactionsGUI(void* a_clbkArg,int64_t a_err,const std::string& a_task,const std::string& a_result);
        void TaskDoneUploadGUI(void* a_clbkArg,int64_t a_err,const std::string& a_task,const std::string& a_result);
    public:
        void TaskDoneOverrviewGUI(void* a_clbkArg,int64_t a_err,const std::string& a_task,const std::string& a_result);
    private:
        void TaskDonePurchasedGUI(void* a_clbkArg,int64_t a_err,const std::string& a_task,const std::string& a_result);

        void TaskDoneFuncGUI3(void* clbkArg,int64_t err,const std::string& task,const fc::variant& result);

        void TaskDoneBrowseContentGUI3(void* a_clbkArg,int64_t a_err,const std::string& a_task,const fc::variant& a_result);
        void TaskDoneTransactionsGUI3(void* a_clbkArg,int64_t a_err,const std::string& a_task,const fc::variant& a_result);
        void TaskDoneUploadGUI3(void* a_clbkArg,int64_t a_err,const std::string& a_task,const fc::variant& a_result);
        void TaskDoneOverrviewGUI3(void* a_clbkArg,int64_t a_err,const std::string& a_task,const fc::variant& a_result);
        void TaskDonePurchasedGUI3(void* a_clbkArg,int64_t a_err,const std::string& a_task,const fc::variant& a_result);

        
        
        
        void ManagementBrowseContentGUI();
        void ManagementTransactionsGUI();
        void ManagementUploadGUI();
        void ManagementOverviewGUI();
        void ManagementPurchasedGUI();

        
        
        void UpdateLockedStatus();

        void CliCallbackFnc(void*arg,const std::string& task);
        int GetDigitalContentsFromVariant(DCT::DIG_CONT_TYPES a_type,
                                         std::vector<SDigitalContent>& acContents,
                                         const fc::variant& contents_var);

        void DisplayWalletContentGUI();

        void SetPassword(void* a_owner, void* a_str_ptr);
        
        
    protected slots:
        void CurrentUserChangedSlot(const QString&);

    protected slots:/* Instead of these one line slots
                     *, probably should be used lambda functions?
                     * Is it possible to do?
                     */
        void AboutSlot();
        void HelpSlot();
        void InfoSlot();

        void ShowWalletContentSlot();

        void ConnectSlot();
        void ImportKeySlot();
        void LockSlot();
        void UnlockSlot();
        void OpenCliWalletDlgSlot();
        void OpenInfoDlgSlot();

        void ShowDetailsOnDigContentSlot(SDigitalContent dig_cont);

    protected:
        virtual void moveEvent(QMoveEvent *) override ;

    protected:
        class QVBoxLayout*   m_pCentralAllLayout;
        class QHBoxLayout*   m_pMenuLayout;
        CentralWigdet*       m_pCentralWidget;

        QMenuBar *          m_barLeft ;
        QMenuBar *          m_barRight;
        QMenu*              m_pMenuFile;
        QMenu*              m_pMenuSetting;
        QMenu*              m_pMenuHelpL;
        QMenu*              m_pMenuContent;
        QMenu*              m_pMenuHelpR;
        QMenu*              m_pMenuCreateTicket;
        QMenu*              m_pMenuDebug;
        QMenu*              m_pMenuTempFunctions;
        QAction             m_ActionExit;
        QAction             m_ActionConnect;
        QAction             m_ActionAbout;
        QAction             m_ActionInfo;
        QAction             m_ActionHelp;
        QAction             m_ActionWalletContent;
        QAction             m_ActionLock;
        QAction             m_ActionUnlock;
        QAction             m_ActionImportKey;
        QAction             m_ActionOpenCliWallet;
        QAction             m_ActionOpenInfoDlg;
        ConnectDlg          m_ConnectDlg;
        TextDisplayDialog   m_info_dialog;
        
        
        QVBoxLayout             m_main_layout;
        QLabel                  m_num_acc_or_error_label;
        int                     m_nError;
        std::string             m_error_string;

        bool                m_locked;

        decent::gui::tools::RichDialog m_import_key_dlg;

        CliWalletDlg                        m_cCliWalletDlg;
        std::string                         m_cli_line;

        QString                            m_cqsPreviousFilter;
        QTextEdit*                          m_pInfoTextEdit;
        CliWalletDlg*                        m_pcInfoDlg;

        std::vector<SDigitalContent> m_vcDigContent;
        std::vector<SDigitalContent> m_vcDigContentOld;
        int                     m_nConnected;
        int                     m_nUserComboTriggeredInGui;
        SConnectionStruct   m_wdata2;

        PasswordDialog      m_SetPasswordDialog;
        PasswordDialog      m_UnlockDialog;

        int                 m_nJustConnecting;

        QString             m_default_stylesheet;

        ContentDetailsGeneral m_dig_cont_detailsGenDlg;
        ContentDetailsBougth m_dig_cont_detailsBougDlg;

        std::vector<std::string>        m_user_ids;
    };

}

#endif // MAINWINDOW_GUI_WALLET_H
