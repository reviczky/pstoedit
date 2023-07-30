#include "PstoeditQtGui.h"
#include <QtWidgets/QApplication>
#include <QMessageBox>
#include <sstream>

#include "pstoedit.h"
#include "poptions.h"

/* Returns number of characters successfully written  */
static int write_callback_to_log_widget(void* cb_data, const char* text, int length)
{
    if ((length > 0) && cb_data && text) {   
        PstoeditQtGui* w_ptr = (PstoeditQtGui*)cb_data;
        const char * text_copy = cppstrndup(text, length);
        const QString qtext(text_copy);
        if (w_ptr->debugStream) {
                *(w_ptr->debugStream) << text_copy;
        }
        w_ptr->logText(qtext);
        delete[] text_copy;
        // update GUI window - especially Log Window
        QCoreApplication::processEvents();
    }
    return length;
}


int main(int argc, char* argv[])
{    
    const bool with_debug_stream = false;
    std::ofstream* debugStream = nullptr;
    
    QApplication a(argc, argv);
    if (with_debug_stream) {
        debugStream = new std::ofstream ("pstoedit.debug.txt");
    }

    PstoeditQtGui theGui(argc, argv, cout, debugStream); 
    
    callbackBuffer wbuf(&theGui, write_callback_to_log_widget);
    std::streambuf* coutbuffer = cout.rdbuf(&wbuf);
    std::streambuf* cerrbuffer = cerr.rdbuf(&wbuf);
    theGui.show();

    bool OK = pstoedit_checkversion(pstoeditdllversion);
    if (!OK) {
        std::stringstream message;
        message << "Version of pstoedit.dll does not match expected version in GUI - cannot proceed. Version expected is: " << pstoeditdllversion << endl;
        cerr << message.str();
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText(QString(message.str().c_str()));
        msgBox.exec();
    } else {
        setPstoeditsetDLLUsage(true);
        OK = theGui.checkGhostscript();
        if (!OK) {
            QMessageBox msgBox;
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.setText(QString("Didn't find any installation of Ghostscript. Please install Ghostscript first."));
            msgBox.exec();
        }
    }

    if (OK)  {
        useCoutForDiag(0); // false means "use cerr"
        setPstoeditOutputFunction(&theGui, write_callback_to_log_widget);
        ProgramOptions* globaloptions = getProgramOptions();
        assert(globaloptions);
        (void) globaloptions->parseoptions(cerr, argc, argv);  
        // FIXME what if a filename was already given ???
        theGui.setProgramOptions(globaloptions);
        loadpstoeditplugins(argv[0], cerr, theGui.Verbose() /* verbose */);
        theGui.mapoptionstodialog();
    }
        
    const int result = a.exec();
    unloadpstoeditplugins();

    // detach std streams from GUI
    cout.rdbuf(coutbuffer);
    cerr.rdbuf(cerrbuffer);
    return result;
}
