#ifndef CCOMBOBOXDELEGATE_HPP
#define CCOMBOBOXDELEGATE_HPP

#include <QStyledItemDelegate>

class CComboBoxDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit CComboBoxDelegate(QObject *parent = 0);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;

signals:

public slots:

};

#endif // CCOMBOBOXDELEGATE_HPP
