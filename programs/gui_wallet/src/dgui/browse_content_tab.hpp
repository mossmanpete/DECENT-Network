/*
 *	File: browse_content_tab.hpp
 *
 *	Created on: 11 Nov 2016
 *	Created by: Davit Kalantaryan (Email: davit.kalantaryan@desy.de)
 *
 *  This file implements ...
 *
 */
#ifndef BROWSE_CONTENT_TAB_H
#define BROWSE_CONTENT_TAB_H

#include <QWidget>
#include <QTableWidget>
#include <QVBoxLayout>
#include <vector>
#include <string>
#include "qt_commonheader.hpp"
#include <QLineEdit>
#include <QHBoxLayout>
#include <QComboBox>





namespace gui_wallet
{

    // DCF stands for Digital Contex Fields
namespace DCF {enum DIG_CONT_FIELDS{TIME,SYNOPSIS,RATING,SIZE,PRICE,LEFT,NUM_OF_DIG_CONT_FIELDS};}

// ST stands for search type
namespace ST{
    enum STtype{URI_start,author,content};
    static const char* s_vcpcSearchTypeStrs[] = {"URI_start","author","content"};
}



class BTableWidget : public QTableWidget
{
    Q_OBJECT
public:
    BTableWidget(int a , int b) : QTableWidget(a,b)
    {
         this->setMouseTracking(true);
    };

    virtual void mouseMoveEvent(QMouseEvent * event);
public:
signals:
    void mouseMoveEventDid();
};







class Browse_content_tab : public QWidget
{
    friend class CentralWigdet;
    Q_OBJECT
public:
    Browse_content_tab();
    virtual ~Browse_content_tab();

    void SetDigitalContentsGUI(const std::vector<SDigitalContent>& contents);
    QString getFilterText()const;

    void Connects();
    int green_row;

public:
signals:
    void ShowDetailsOnDigContentSig(SDigitalContent get_cont_str);

public slots:
    void doRowColor();

protected:
    void PrepareTableWidgetHeaderGUI();
    void DigContCallback(_NEEDED_ARGS2_);
    virtual void resizeEvent ( QResizeEvent * a_event );
    void ArrangeSize();

protected:
    QVBoxLayout     m_main_layout;
    QHBoxLayout     m_search_layout;
    //QTableWidget    m_TableWidget; // Should be investigated
    BTableWidget*    m_pTableWidget;
    //int              m_nNumberOfContentsPlus1;
    QLineEdit       m_filterLineEdit;
    QComboBox       m_searchTypeCombo;
};


}

#endif // BROWSE_CONTENT_TAB_H
