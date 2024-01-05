#include "customgraphics.h"

customMap::customMap(QWidget *parent)
    : QGraphicsView(parent) {
    setDragMode(QGraphicsView::ScrollHandDrag);
}

void customMap::wheelEvent(QWheelEvent *event) {
    double maxScale = 2.0;
    double minScale = 0.65;
    double scaleFactor = 1.1;
    if (event->modifiers() == Qt::ControlModifier) {
        double currentScaleFactor = transform().m11();

        double newScaleFactor;

        if (event->angleDelta().y() > 0) {
            // zoom in
            newScaleFactor = currentScaleFactor * scaleFactor;
            if (newScaleFactor > maxScale) {
                newScaleFactor = maxScale;
            }
        } else {
            // zoom out
            newScaleFactor = currentScaleFactor / scaleFactor;
            if (newScaleFactor < minScale) {
                newScaleFactor = minScale;
            }
        }
        scale(newScaleFactor / currentScaleFactor, newScaleFactor / currentScaleFactor);
        event->accept();
    }
    else {
        QGraphicsView::wheelEvent(event);
    }
}


customEllipse::customEllipse(qreal x, qreal y, qreal width, qreal height,
                             const QString& name, const QString& label, bool ra, qreal Sx, qreal Sy,
                             QGraphicsItem *parent)
    : QGraphicsEllipseItem(x, y, width, height, parent), m_name(name), m_label(label), ra(ra), m_x(Sx), m_y(Sy)
{
    setAcceptHoverEvents(true);
}


void customEllipse::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    setRect(rect().x() - 5, rect().y() - 5, rect().width() + 10, rect().height() + 10);
    QToolTip::showText(event->screenPos(), m_name);
    setBrush(QColor(pen().color()));
    QGraphicsEllipseItem::hoverEnterEvent(event);
    update();
}

void customEllipse::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    QGraphicsEllipseItem::hoverLeaveEvent(event);
    setRect(rect().x() + 5, rect().y() + 5, rect().width() - 10, rect().height() - 10);
    QToolTip::hideText();
    setBrush(Qt::transparent);
    update();

}

customPopupWidget::customPopupWidget(const QString &text, const QString& label, bool ra, QWidget *parent)
    : m_label(new QLabel("caption", this)), ra(ra), QWidget(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(200, 100);

    m_text = new QTextEdit(this);
    m_text->setPlainText("\n" + text);
    m_text->setReadOnly(true);
    m_text->setGeometry(10, 10, width() - 20, height() - 20);

    m_label = new QLabel("Caption", this);
    m_label->setStyleSheet("background-color: #f0f0f0; border-radius: 5px;");
    m_label->setAlignment(Qt::AlignCenter);
    m_label->setGeometry(0, 0, width(), 20); //

    setStyleSheet("customPopupWidget {"
                  "    border: 2px solid black;"
                  "    border-radius: 5px;"
                  "    background-color: white;"
                  "}");

    m_label->setText(label);
}

customPopupWidget::customPopupWidget(const QString &text, const QString &label, QWidget *parent)
    : m_label(new QLabel("caption", this)), ra(0), QWidget(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(200, 100);

    m_text = new QTextEdit(this);
    m_text->setPlainText("\n" + text);
    m_text->setReadOnly(true);
    m_text->setGeometry(10, 10, width() - 20, height() - 20);

    m_label = new QLabel("Caption", this);
    m_label->setStyleSheet("background-color: #f0f0f0; border-radius: 5px;");
    m_label->setAlignment(Qt::AlignCenter);
    m_label->setGeometry(0, 0, width(), 20); //

    setStyleSheet("customPopupWidget {"
                  "    border: 2px solid black;"
                  "    border-radius: 5px;"
                  "    background-color: white;"
                  "}");

    m_label->setText(label);
}


void customEllipse::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (!m_popupWidget)
    {
        QPoint pos = event->screenPos();
        m_popupWidget = new customPopupWidget(m_name, m_label, ra);
        m_popupWidget->move(pos);
        m_popupWidget->show();
    }
    else
    {
        m_popupWidget->close();
        delete m_popupWidget;
        m_popupWidget = nullptr;
    }

    QGraphicsEllipseItem::mousePressEvent(event);
}


customScene::customScene(QObject *parent)
    : QGraphicsScene(parent) {}

void customScene::addNodes(customEllipse *node)
{
    nodes.push_back(node);
}


void customScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (startPress)
    {
        foreach (customEllipse* node, nodes)
        {
            if (node->contains(node->mapFromScene(event->scenePos())))
            {
                emit startClick(node->get_name());
                startPress = 0;
            }
        }
    }
    else if (endPress)
    {
        foreach (customEllipse* node, nodes)
        {
            if (node->contains(node->mapFromScene(event->scenePos())))
            {
                emit endClick(node->get_name());
                endPress = 0;
            }
        }
    }
    QGraphicsScene::mousePressEvent(event);
}



void customButton::keyPressEvent(QKeyEvent *event)
{
    if (this->hasFocus() && (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter))
        emit enterKeyPressed(this);

    QPushButton::keyPressEvent(event);
}



void customPath::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
    if (!m_popupWidget)
    {
        QPoint pos = event->screenPos();
        m_popupWidget = new customPopupWidget(m_name, m_label);
        m_popupWidget->move(pos);
        m_popupWidget->show();
    }
    else
    {
        m_popupWidget->close();
        delete m_popupWidget;
        m_popupWidget = nullptr;
    }
    QGraphicsPathItem::mousePressEvent(event);
}
