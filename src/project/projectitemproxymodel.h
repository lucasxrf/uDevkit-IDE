#ifndef PROJECTITEMPROXYMODEL_H
#define PROJECTITEMPROXYMODEL_H

#include <QSortFilterProxyModel>

class Project;

class ProjectItemProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit ProjectItemProxyModel(Project *project);

public slots:
    void setHiddenFilter(const QRegExp &regExp);
    void setHiddenFilter(const QString &pattern);
    void enableHiddenFilter(bool enable=true);

    void setShowFilter(const QRegExp &regExp);
    void setShowFilter(const QString &pattern);

    // QSortFilterProxyModel interface
protected:
    virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
    virtual bool filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const;

protected:
    Project *_project;
    QRegExp _hiddenFilter;
    QRegExp _showFilter;
    bool _enabledHiddenFilter;

    // QSortFilterProxyModel interface
protected:
    virtual bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const;
};

#endif // PROJECTITEMPROXYMODEL_H
