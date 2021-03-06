#ifndef CODEEDITORSETTINGS_H
#define CODEEDITORSETTINGS_H

#include "settingspage.h"

#include <QFontComboBox>
#include <QSpinBox>

#include "editor/codeeditor/codeeditor.h"

class CodeEditorSettings : public SettingsPage
{
    Q_OBJECT
public:
    CodeEditorSettings();

protected slots:
    void updateTest();

    // SettingsPage interface
protected:
    virtual void execCommit();
    virtual void createWidgets();

    QFontComboBox *_fontComboBox;
    QSpinBox *_fontSizeSpinBox;
    CodeEditor *_editorTest;
};

#endif // CODEEDITORSETTINGS_H
