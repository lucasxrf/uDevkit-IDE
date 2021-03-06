#include "searchreplacewidget.h"

#include <QBoxLayout>
#include <QDebug>
#include <QFocusEvent>
#include <QRegularExpression>

SearchReplaceWidget::SearchReplaceWidget(QWidget *parent) : QWidget(parent)
{
    _editor = Q_NULLPTR;

    createWidgets();
    setEditor(Q_NULLPTR);
    setFocusPolicy(Qt::StrongFocus);
}

void SearchReplaceWidget::setEditor(Editor *editor)
{
    _editor = editor;

    if (!_editor || !_editor->hasResearch())
    {
        setEnabled(false);
        return;
    }

    setEnabled(true);
    _regexpCheckbox->setEnabled(_editor->hasRegExp());
}

void SearchReplaceWidget::activateResearch()
{
    if (parentWidget() && parentWidget()->parentWidget())
    {
        parentWidget()->show();
        parentWidget()->raise();
        parentWidget()->parentWidget()->show();
        parentWidget()->parentWidget()->raise();
    }
    show();
    _searchLineEdit->setFocus();
    _searchLineEdit->selectAll();
}

void SearchReplaceWidget::activateReplace()
{
    if (parentWidget() && parentWidget()->parentWidget())
        parentWidget()->parentWidget()->show();
    show();
    _replaceLineEdit->setFocus();
    _replaceLineEdit->selectAll();
}

Editor::SearchFlags SearchReplaceWidget::flags()
{
    Editor::SearchFlags mflags;
    if (_regexpCheckbox->isEnabled() && _regexpCheckbox->isChecked())
        mflags |= Editor::RegExpMode;
    if (_caseSensitivityCheckbox->isEnabled() && _caseSensitivityCheckbox->isChecked())
        mflags |= Editor::CaseSensitive;
    return mflags;
}

void SearchReplaceWidget::upadteSearch()
{
    if (!_editor)
        return;

    int found = _editor->search(_searchLineEdit->text(), flags());
    if (found == 0 && _regexpCheckbox->isEnabled() && _regexpCheckbox->isChecked())
    {
        QRegularExpression regexp(_searchLineEdit->text());
        if (!regexp.isValid())
        {
            _statusLabel->setText("<span style='color:red;'>" + regexp.errorString() + "</span>");
            return;
        }
    }
    _statusLabel->setText(tr("%n occurence(s) found", "", found));
}

void SearchReplaceWidget::searchNext()
{
    if (!_editor)
        return;
    _editor->searchNext();
}

void SearchReplaceWidget::searchPrev()
{
    if (!_editor)
        return;
    _editor->searchPrev();
}

void SearchReplaceWidget::searchAll()
{
    if (!_editor)
        return;
    _editor->searchSelectAll();
}

void SearchReplaceWidget::replaceNext()
{
    if (!_editor)
        return;
    _editor->replace(_replaceLineEdit->text(), flags());
}

void SearchReplaceWidget::replacePrev()
{
    if (!_editor)
        return;
    _editor->replace(_replaceLineEdit->text(), flags(), false);
}

void SearchReplaceWidget::replaceAll()
{
    if (!_editor)
        return;

    _editor->replaceAll(_replaceLineEdit->text(), flags());
}

void SearchReplaceWidget::createWidgets()
{
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setMargin(0);

    // search and next prev buttons
    QHBoxLayout *searchLineLayout = new QHBoxLayout();
    searchLineLayout->setMargin(0);
    _searchLineEdit = new QLineEdit();
    _searchLineEdit->setPlaceholderText(tr("search"));
    _searchLineEdit->installEventFilter(this);
    searchLineLayout->addWidget(_searchLineEdit);
    connect(_searchLineEdit, &QLineEdit::textChanged, this, &SearchReplaceWidget::upadteSearch);
    connect(_searchLineEdit, &QLineEdit::returnPressed, this, &SearchReplaceWidget::searchNext);

    _prevButton = new QToolButton();
    _prevButton->setText("<");
    _prevButton->setToolTip(tr("Search backward and select"));
    _prevButton->setIcon(QIcon(":/icons/img/dark/icons8-previous.png"));
    searchLineLayout->addWidget(_prevButton);
    connect(_prevButton, &QToolButton::clicked, this, &SearchReplaceWidget::searchPrev);

    _nextButton = new QToolButton();
    _nextButton->setText(">");
    _nextButton->setToolTip(tr("Search forward and select"));
    _nextButton->setIcon(QIcon(":/icons/img/dark/icons8-next.png"));
    searchLineLayout->addWidget(_nextButton);
    connect(_nextButton, &QToolButton::clicked, this, &SearchReplaceWidget::searchNext);

    _allButton = new QToolButton();
    _allButton->setText("*");
    _allButton->setToolTip(tr("Search and select all"));
    _allButton->setIcon(QIcon(":/icons/img/dark/icons8-search.png"));
    searchLineLayout->addWidget(_allButton);
    connect(_allButton, &QToolButton::clicked, this, &SearchReplaceWidget::searchAll);

    layout->addLayout(searchLineLayout);

    // search and next prev buttons
    QHBoxLayout *replaceLineLayout = new QHBoxLayout();
    replaceLineLayout->setMargin(0);
    _replaceLineEdit = new QLineEdit();
    _replaceLineEdit->setPlaceholderText(tr("replace"));
    _replaceLineEdit->installEventFilter(this);
    replaceLineLayout->addWidget(_replaceLineEdit);
    connect(_replaceLineEdit, &QLineEdit::returnPressed, this, &SearchReplaceWidget::replaceNext);

    _replacePrevButton = new QToolButton();
    _replacePrevButton->setText("<");
    _replacePrevButton->setToolTip(tr("Replace previous"));
    _replacePrevButton->setIcon(QIcon(":/icons/img/dark/icons8-back-to.png"));
    replaceLineLayout->addWidget(_replacePrevButton);
    connect(_replacePrevButton, &QToolButton::clicked, this, &SearchReplaceWidget::replacePrev);

    _replaceNextButton = new QToolButton();
    _replaceNextButton->setText(">");
    _replaceNextButton->setToolTip(tr("Replace next"));
    _replaceNextButton->setIcon(QIcon(":/icons/img/dark/icons8-next-page.png"));
    replaceLineLayout->addWidget(_replaceNextButton);
    connect(_replaceNextButton, &QToolButton::clicked, this, &SearchReplaceWidget::replaceNext);

    _replaceAllButton = new QToolButton();
    _replaceAllButton->setText("*");
    _replaceAllButton->setToolTip(tr("Replace all"));
    _replaceAllButton->setIcon(QIcon(":/icons/img/dark/icons8-find-and-replace.png"));
    replaceLineLayout->addWidget(_replaceAllButton);
    connect(_replaceAllButton, &QToolButton::clicked, this, &SearchReplaceWidget::replaceAll);

    layout->addLayout(replaceLineLayout);

    // options
    QHBoxLayout *optionsLineLayout = new QHBoxLayout();
    optionsLineLayout->setMargin(0);

    _regexpCheckbox = new QCheckBox(tr("regexp"));
    _regexpCheckbox->setChecked(true);
    optionsLineLayout->addWidget(_regexpCheckbox);
    connect(_regexpCheckbox, &QCheckBox::stateChanged, this, &SearchReplaceWidget::upadteSearch);

    _caseSensitivityCheckbox = new QCheckBox(tr("case sensitive"));
    _caseSensitivityCheckbox->setChecked(true);
    optionsLineLayout->addWidget(_caseSensitivityCheckbox);
    connect(_caseSensitivityCheckbox, &QCheckBox::stateChanged, this, &SearchReplaceWidget::upadteSearch);
    optionsLineLayout->addStretch();

    layout->addLayout(optionsLineLayout);

    // status
    _statusLabel = new QLabel();
    _statusLabel->setTextFormat(Qt::RichText);
    _statusLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    _statusLabel->setCursor(Qt::IBeamCursor);
    layout->addWidget(_statusLabel);

    setLayout(layout);

    // tab order
    setTabOrder(_searchLineEdit, _replaceLineEdit);
}

bool SearchReplaceWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == _searchLineEdit && event->type() == QEvent::FocusIn && !_searchLineEdit->text().isEmpty())
        upadteSearch();
    if (watched == _replaceLineEdit && event->type() == QEvent::FocusIn && !_searchLineEdit->text().isEmpty())
        upadteSearch();
    return QObject::eventFilter(watched, event);
}
