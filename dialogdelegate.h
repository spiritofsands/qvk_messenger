#ifndef DIALOGDELEGATE_H
#define DIALOGDELEGATE_H

#include <QStyledItemDelegate>
#include <QMap>

class DialogDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    DialogDelegate(QObject *parent = 0);

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const;
    QSize sizeHint (const QStyleOptionViewItem &option,
                    const QModelIndex &index ) const;

    enum dialogDisplayRoles{
        NAME_ROLE = Qt::UserRole,
        LAST_MESSAGE_ROLE,
        READ_STATE_ROLE,
        PROFILE_ID_ROLE
    };

private:
    const double paddingConst = 0.1;

    const QSize avatarSize = QSize(50, 50);

    const int minHeight = avatarSize.height()
                            * (1 + paddingConst*4);

};

#endif // DIALOGDELEGATE_H
