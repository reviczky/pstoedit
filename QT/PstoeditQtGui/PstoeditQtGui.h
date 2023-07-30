#pragma once

#include <QtWidgets/QMainWindow>
#include <QFormLayout>
#include <QStringList>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QFuture>
#include <QtNetwork/QNetworkAccessManager>
#include <map>


#include "ui_PstoeditQtGui.h"
#include "pstoedit.h"
#include "pstoeditoptions.h"


class WidgetToOptionBase {
public:
    WidgetToOptionBase(OptionBase* theOption_p, int tab_index_p) : theOption(theOption_p), tab_index(tab_index_p) {}
    virtual void transferValue() = 0;
    OptionBase* const theOption;
    const int tab_index;    // the index of the tab where this option is shown in. 
                            // Needed because we need to delete the driver specific tab 
                            // at some point and then we need to remove it from the widgettooptions map.
};
class CheckboxToOption : public WidgetToOptionBase {
public:
    CheckboxToOption(QCheckBox* theWidget_p, OptionBase* theOption_p, int tab_index_p) : WidgetToOptionBase(theOption_p, tab_index_p), theWidget(theWidget_p) {}
    virtual void transferValue() {
        theOption->copyValueFromString(theWidget->isChecked() ? "1" : "0");
    }
private:
    QCheckBox* const theWidget;
};

class LineEditToOption : public WidgetToOptionBase {
public:
    LineEditToOption(QLineEdit* theWidget_p, OptionBase* theOption_p, int tab_index_p) : WidgetToOptionBase(theOption_p, tab_index_p), theWidget(theWidget_p) {}
    virtual void transferValue() {
        QString value = theWidget->text();
        theOption->copyValueFromString(value.toStdString().c_str());
    }
private:
    const QLineEdit* const theWidget;
};

class ComboBoxToOption : public WidgetToOptionBase {
public:
    ComboBoxToOption(QComboBox* theWidget_p, OptionBase* theOption_p, int tab_index_p) : WidgetToOptionBase(theOption_p, tab_index_p), theWidget(theWidget_p) {}
    virtual void transferValue() {
        QString value = theWidget->currentText();
        theOption->copyValueFromString(value.toStdString().c_str());
    }
private:
    const QComboBox* const theWidget;
};

class PstoeditQtGui : public QMainWindow
{
    Q_OBJECT
    enum class ButtonState { FullyDisabled, StartEnabled, StopEnabled };
public:
    PstoeditQtGui(int argc_p, char ** argv_p, std::ostream& logStream_p, std::ostream* debugStream_p, QWidget* parent = Q_NULLPTR);
    ~PstoeditQtGui() { }
    bool Verbose() { return verbose; }
    bool checkGhostscript();
    void setProgramOptions(ProgramOptions* param);
    void mapoptionstodialog();
    void logText(const QString& text) {
        ui.logWidget->insertPlainText(text);
        ui.logWidget->ensureCursorVisible();
    }
    std::ostream& logStream;
    std::ostream* debugStream = nullptr;

private:
 
    void fillTabWithOptions(int new_tab_index, ProgramOptions* options, int sheet, unsigned int opttype, QFormLayout* layout, const char * anchorprefix);
    int  createOptionTab(ProgramOptions* options, int sheet, const char* const sheetName, const char * anchorprefix);
    void transferOptionValues();
    void removeTabFromMappingTable(int tab_to_be_removed);
    
    int  runConversion();
    void startConversion();
    void cancelConversion();
    void startOrStopConversion();
    void updateStartStopButton(ButtonState newState);
    void quit();
    void updateOutputSuffix();
    void getInputFiles();
  
  
    // Actions used in menues
    void OpenGUIHelpDocument() ;
   // void CheckForUpdates();
    void About();
    void SupportPstoeditDevelopmentandMaintenance();
 

    QNetworkAccessManager HTTPmanager;
    void ReplyFinished(class QNetworkReply* reply);
    void CheckForUpdates();

    Ui::PstoeditQtGuiClass ui;
    bool verbose = false;
    QStringList files;
    QComboBox* formatSelector = nullptr;
    QComboBox* versionSelector = nullptr;
    QLineEdit* inputFilesOutput = nullptr;
    QLineEdit* outputSuffix = nullptr;
    QPushButton* startStopButton = nullptr;
    ButtonState buttonState = ButtonState::FullyDisabled;
    QFuture<int> future_for_conversion;
    bool continue_conversion = true;
    std::list<WidgetToOptionBase* > widgetsToOptions;
    ProgramOptions* globaloptions = nullptr;
    ProgramOptions* driverOptions = nullptr;
    const int argc;
    char** const argv;
    unsigned int driverSpecificOptionTab = 0;
    std::map<int, std::string> gsversions;
    int highestVersion = 0;
    std::string highestVersionString;
    QStringList versionSelectorItems;
    QString UrlOfPstoeditHtml;
};
