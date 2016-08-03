#include "dialogdelegate.h"

#include "mainwindow.h"

#include <QPainter>

DialogDelegate::DialogDelegate(QObject *parent)
 : QStyledItemDelegate(parent)
{
    //nope
}

void DialogDelegate::paint(QPainter *painter,
           const QStyleOptionViewItem &constOption,
           const QModelIndex &index) const
{
    if (!index.isValid())
        return;

    painter->save();

    QStyleOptionViewItem option(constOption);
    initStyleOption(&option, index);

    QRect listItemRect(option.rect);

    if(option.state & QStyle::State_Selected){
        painter->fillRect(option.rect, option.palette.color(QPalette::Highlight));
    }
    else if (!index.data(READ_STATE_ROLE).toBool()) { //unread
        QColor unreadColor = option.palette.color(QPalette::Highlight).lighter(175);
        painter->fillRect(option.rect, unreadColor);
    }

    //avatar
    QPixmap avatar = QPixmap(qvariant_cast<QPixmap>(index.data(Qt::DecorationRole)));
    QString name = index.data(NAME_ROLE).toString();
    QString lastMessage = index.data(LAST_MESSAGE_ROLE).toString();

    int padding = listItemRect.height()*paddingConst;

    listItemRect.adjust(padding, padding,
                  -padding, -padding);

    QRect avatarRect(listItemRect.topLeft(), avatarSize);
    painter->drawPixmap(avatarRect, avatar);


    //name
    QRect nameRectMax = listItemRect;
    nameRectMax.adjust(avatarRect.width() + padding, 0, 0, 0);

    QFont font;
    font.setBold(true);
    painter->setFont(font);
    QFontMetrics fontMetrics = painter->fontMetrics();
    QRect nameRect = fontMetrics.boundingRect(
                nameRectMax,
                Qt::AlignTop | Qt::TextWordWrap,
                name);
    painter->drawText(nameRect, name);


    //last message
    QRect lastMessageRectMax = nameRectMax
            .adjusted(0, nameRect.height() + padding, 0, 0);
    font.setBold(false);
    painter->setFont(font);
    fontMetrics = painter->fontMetrics();
    QRect lastMessageRect = fontMetrics.boundingRect(
                lastMessageRectMax,
                Qt::AlignTop | Qt::TextWordWrap,
                lastMessage);

    painter->drawText(lastMessageRect, lastMessage);

    painter->restore();
}

QSize DialogDelegate::sizeHint (const QStyleOptionViewItem &option,
                const QModelIndex &/*index*/ ) const
{
    return QSize(option.rect.width(), minHeight);
}
