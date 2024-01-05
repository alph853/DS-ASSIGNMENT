#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("K shortest paths and Maximum flow problem");

    scene = new customScene(this);
    Operate = std::make_unique<operate>(140);



    ui->spbutton->setEnabled(0);
    ui->flowbutton->setEnabled(0);

    ui->nextbutton->setEnabled(0);
    ui->prevbutton->setEnabled(0);

    ui->startlabel->setText("");
    ui->endlabel->setText("");

    ui->chooseDes->setPlaceholderText("  Choose destination");
    ui->chooseStart->setPlaceholderText("  Choose start location");


    ui->map->setScene(scene);
    QPixmap pixmap(":/images/map.png");
    QGraphicsPixmapItem *item = scene->addPixmap(pixmap);
    QPixmap pixmap2(":/images/grid.jpg");
    QGraphicsPixmapItem *itemGrid = scene->addPixmap(pixmap2);
    item->setVisible(true);
    itemGrid->setVisible(false);


    ui->startlabel->setText("Please select start location.");
    ui->endlabel->setText("Please select destination.");

    scene->setSceneRect(item->boundingRect());
    qreal sceneWidth = scene->width();
    qreal sceneHeight = scene->height();

    qreal pixmapWidth = item->pixmap().width();
    qreal pixmapHeight = item->pixmap().height();

    qreal scaleX = sceneWidth / pixmapWidth;
    qreal scaleY = sceneHeight / pixmapHeight;
    qreal scaleFactor = qMin(scaleX, scaleY);
    item->setScale(scaleFactor);

    view = new QGraphicsView(scene);
    view->setRenderHint(QPainter::Antialiasing);

    color = {
        QColor(13, 85, 255),
        Qt::blue,
        QColor(255, 162, 41),
        QColor(219, 122, 24),
        QColor(219, 65, 9),
        QColor(168, 50, 7),
        QColor(160, 42, 168),
        QColor(133, 18, 150),
        QColor(150, 13, 86),
        QColor(150, 16, 61),
        Qt::red,
        QColor(150, 6, 30),
        QColor(13, 22, 84),
        QColor(6, 11, 41)
    };

    addNodesToScene();

    connect(ui->chooseStart, &QLineEdit::textChanged, this, &MainWindow::textStart);
    connect(ui->chooseDes, &QLineEdit::textChanged, this, &MainWindow::textEnd);

    connect(scene, &customScene::startClick, this, &MainWindow::set_startText);
    connect(scene, &customScene::endClick, this, &MainWindow::set_endText);

    connect(ui->mapToggle, &QPushButton::clicked, this, [=]() {
        item->setVisible(!item->isVisible());
        itemGrid->setVisible(!itemGrid->isVisible());
    });

    connect(ui->spbutton, &customButton::enterKeyPressed, this, &MainWindow::onEnterPressed);
    connect(ui->flowbutton, &customButton::enterKeyPressed, this, &MainWindow::onEnterPressed);
    connect(ui->buttonStart, &customButton::enterKeyPressed, this, &MainWindow::onEnterPressed);
    connect(ui->buttonEnd, &customButton::enterKeyPressed, this, &MainWindow::onEnterPressed);
    connect(ui->resetbutton, &customButton::enterKeyPressed, this, &MainWindow::onEnterPressed);
    connect(ui->switchbutton, &customButton::enterKeyPressed, this, &MainWindow::onEnterPressed);
    connect(ui->nextbutton, &customButton::enterKeyPressed, this, &MainWindow::onEnterPressed);
    connect(ui->prevbutton, &customButton::enterKeyPressed, this, &MainWindow::onEnterPressed);


    connect(ui->spbutton, &customButton::pressed, this, &MainWindow::on_spbutton_clicked);
    connect(ui->flowbutton, &customButton::pressed, this, &MainWindow::on_flowbutton_clicked);
    connect(ui->buttonStart, &customButton::pressed, this, &MainWindow::on_buttonStart_clicked);
    connect(ui->buttonEnd, &customButton::pressed, this, &MainWindow::on_buttonEnd_clicked);
    connect(ui->resetbutton, &customButton::pressed, this, &MainWindow::on_resetbutton_clicked);


    QWidget::setTabOrder(ui->chooseStart, ui->chooseDes);
    QWidget::setTabOrder(ui->chooseDes, ui->spbutton);
    QWidget::setTabOrder(ui->spbutton, ui->flowbutton);

    ui->prevbutton->hide();
    ui->nextbutton->hide();

    this->setStyleSheet("background-color: rgb(245,250,255);");

    QStringList nodes;
    for(const auto& x : Operate->get_nodeList())
    {
        QString name = "  " + QString::fromStdString(x.name);
        if (name.contains("Node"))
            nodes.append(name);
        else nodes << name;
    }

    QCompleter* completer = new QCompleter(nodes);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    ui->chooseStart->setCompleter(completer);


    QCompleter* completerDes = new QCompleter(nodes);
    completerDes->setCaseSensitivity(Qt::CaseInsensitive);
    completerDes->setCompletionMode(QCompleter::PopupCompletion);
    ui->chooseDes->setCompleter(completerDes);
}

void MainWindow::textStart() {
    disconnect(ui->chooseStart, &QLineEdit::textChanged, this, &MainWindow::textStart);
    auto text =  ui->chooseStart->text();
    if (text == "" || text == " ")
    {
        text = "  ";
        ui->startlabel->setText("Please select start location.");
    }
    else if (text.length() < 2)
        text = "  " + text;
    ui->chooseStart->setText(text);


    source = Operate->nodeToIndex(text.toStdString());

    if (source == -1)
    {
        ui->startlabel->setText("Start location invalid.");
        ui->startlabel->setStyleSheet("color: red");
    }
    else
    {
        ui->startlabel->setText("Start location viable.");
        ui->startlabel->setStyleSheet("color: green");
    }

    connect(ui->chooseStart, &QLineEdit::textChanged, this, &MainWindow::textStart);
    const auto& text2 = ui->chooseDes->text();
    bool enableButtons = (!text.isEmpty() && !text2.isEmpty()) || (text != "  " && text2 != "  ");

    ui->spbutton->setEnabled(enableButtons);
    ui->flowbutton->setEnabled(enableButtons);
    ui->switchbutton->setEnabled(enableButtons);
}

void MainWindow::textEnd()
{
    disconnect(ui->chooseDes, &QLineEdit::textChanged, this, &MainWindow::textEnd);
    auto text =  ui->chooseDes->text();
    if (text == "" || text == " ")
    {
        text = "  ";
        ui->endlabel->setText("Please select destination.");
        ui->endlabel->setStyleSheet("color: solid black");
    }
    else if (text.length() < 2)
        text = "  " + text;

    ui->chooseDes->setText(text);
    end = Operate->nodeToIndex(text.toStdString());

    if (end == -1)
    {
        ui->endlabel->setText("Destination invalid.   ");
        ui->endlabel->setStyleSheet("color: red");
    }
    else
    {
        ui->endlabel->setText("Destination viable.   ");
        ui->endlabel->setStyleSheet("color: green");
    }

    connect(ui->chooseDes, &QLineEdit::textChanged, this, &MainWindow::textEnd);
    const auto& text2 = ui->chooseStart->text();
    bool enableButtons = (!text.isEmpty() && !text2.isEmpty()) || (text != "  " && text2 != "  ");

    ui->spbutton->setEnabled(enableButtons);
    ui->flowbutton->setEnabled(enableButtons);
    ui->switchbutton->setEnabled(enableButtons);
}


void MainWindow::removePath()
{
    for (QGraphicsItem* item : scene->items())
    {
        if (dynamic_cast<QGraphicsPathItem*>(item) || dynamic_cast<customPath*>(item) || dynamic_cast<QGraphicsPolygonItem *>(item))
        {
            scene->removeItem(item);
            delete item;
        }
    }
}


void MainWindow::addStartEnd(int source, int end)
{
    auto node = scene->get_nodes()[source];
    auto pos = node->get_pos();
    auto name = node->get_name();
    auto label = node->get_label();

    startNode = new customEllipse(pos.x() - 15, pos.y() - 15, 30, 30, name, label, 0, 20, 20);
    startNode->setBrush(QColor(Qt::transparent));
    startNode->setPen(QPen(node->pen().color(), 5));

    scene->addItem(startNode);

    auto node1 = scene->get_nodes()[end];
    auto pos1 = node1->get_pos();
    auto name1 = node1->get_name();
    auto label1 = node1->get_label();

    endNode = new customEllipse(pos1.x() - 15, pos1.y() - 15, 30, 30, name1, label1, 0, 20, 20);
    endNode->setBrush(QColor(Qt::transparent));
    endNode->setPen(QPen(node1->pen().color(), 5));

    scene->addItem(endNode);
}


void MainWindow::drawPath(const std::vector<int> &p, double weight, const QString& currentMode)
{
    if (p.size() < 2)
        return;

    removePath();

    auto nodes = scene->get_nodes();

    QPolygonF arrow;
    arrow << QPointF(0, 0) << QPointF(-10, -5) << QPointF(-10, 5);

    auto curr = nodes[p[0]]->get_pos();
    int currNode = p[0];


    const auto& roadName = Operate->get_road();
    const auto& nodeDistance = Operate->get_matrix();
    const auto& nodeList = Operate->get_nodeList();

    QString outStr;
    if (currentMode == "Distance")
    {
        if (weight < 1)
            outStr = QString::number(static_cast<int>(weight*1000)) + "m";
        else outStr = QString::number(weight, 'f', 2) + "km";
    }
    else outStr = QString::number(weight);
    outStr = "Path " + QString::number(index) + '\n' + currentMode + ": " + outStr;

    QPainterPath inner;
    std::vector<QGraphicsPolygonItem*> arrows;
    arrows.reserve(p.size() - 1);

    for(size_t i = 1; i < p.size(); i++)
    {
        const QPointF& next = nodes[p[i]]->get_pos();


        double angle = atan2(next.y() - curr.y(), next.x() - curr.x());
        QPointF midpoint = (next + curr) / 2;
        QTransform transform;
        transform.translate(midpoint.x(), midpoint.y());
        transform.rotateRadians(angle);

        QGraphicsPolygonItem* arrowItem = new QGraphicsPolygonItem(arrow);
        arrowItem->setBrush(QBrush(color[index]));
        arrowItem->setPen(QPen(Qt::transparent));
        arrowItem->setTransform(transform);
        arrows.push_back(arrowItem);

        QString str1;
        if (currentMode == "Distance")
        {
            const double& x = nodeDistance[currNode][p[i]];
            if (x < 1)
                str1 = "Distance: " + QString::number(x * 1000, 'd', 0) + 'm';
            else
                str1 = "Distance: " + QString::number(x, 'f', 2) + "km";
        }
        else str1 = "Capacity: " + QString::number(weight);
        str1 += '\n' + QString::fromStdString(nodeList[currNode].name) +
                " -> " + QString::fromStdString(nodeList[p[i]].name);

        QPainterPath roadPath;
        roadPath.moveTo(curr);
        roadPath.lineTo(next);
        customPath* roadP = new customPath(roadPath, str1,"R. " + QString::fromStdString(roadName[currNode][p[i]]));
        roadP->setPen(QPen(color[index - 1], 4));
        roadP->setToolTip(outStr);

        inner.moveTo(curr);
        inner.lineTo(next);

        scene->addItem(roadP);

        curr = next;
        currNode = p[i];
    }


    QGraphicsPathItem* innerPath = new QGraphicsPathItem(inner);
    innerPath->setPen(QPen(color[index], 3));
    scene->addItem(innerPath);

    for(const auto& arrowItem : arrows)
        scene->addItem(arrowItem);
}


void MainWindow::drawPath(const matrix<int> &paths, const std::vector<double>& weight, const QString& mode)
{
    if (paths.size() < 2)
        return;

    removePath();

    int colIndex = 0;
    auto nodes = scene->get_nodes();

    QPolygonF arrow;
    arrow << QPointF(0, 0) << QPointF(-10, -5) << QPointF(-10, 5);
    std::vector<QGraphicsPolygonItem*> arrows;

    const auto& roadName = Operate->get_road();

    for(size_t i = 0; i < paths.size(); i++)
    {
        QPainterPath path;
        QPainterPath outline;
        const auto& p = paths[i];
        QPointF curr = nodes[p[0]]->get_pos();
        path.moveTo(curr);
        outline.moveTo(curr);

        for(size_t i = 1; i < p.size(); i++)
        {
            QPointF next = nodes[p[i]]->get_pos();

            path.lineTo(next);
            outline.lineTo(next);

            double angle = atan2(next.y() - curr.y(), next.x() - curr.x());
            QPointF midpoint = (next + curr) / 2;
            QTransform transform;
            transform.translate(midpoint.x(), midpoint.y());
            transform.rotateRadians(angle);

            QGraphicsPolygonItem* arrowItem = new QGraphicsPolygonItem(arrow);
            arrowItem->setBrush(QBrush(color[colIndex]));
            arrowItem->setTransform(transform);
            arrowItem->setPen(QPen(Qt::transparent));
            arrows.push_back(arrowItem);
            curr = next;
        }

        QGraphicsPathItem* innerPath = new QGraphicsPathItem(path);
        QGraphicsPathItem* outerPath = new QGraphicsPathItem(outline);

        QString outStr;

        if (mode == "Distance")
        {
            if (weight[i] < 1)
                outStr = QString::number(static_cast<int>(weight[i]*1000)) + "m";
            else outStr = QString::number(weight[i], 'f', 2) + "km";
        }
        else outStr = QString::number(weight[i]);

        outerPath->setToolTip("Path " + QString::number(i + 1) + "\n" + mode + ": " + outStr);
        outerPath->setPen(QPen(color[colIndex], 4));
        colIndex = (colIndex + 1) % color.size();

        innerPath->setPen(QPen(color[colIndex], 3));
        colIndex = (colIndex + 1) % color.size();

        scene->addItem(outerPath);
        scene->addItem(innerPath);
    }
    for(const auto& arrowItem : arrows)
        scene->addItem(arrowItem);
}

void MainWindow::drawMatrix(const matrix<double> &nodeMatrix)
{
    removePath();

    auto node_matrix = nodeMatrix;
    auto nodes = scene->get_nodes();

    QPolygonF arrow;
    arrow << QPointF(0, 0) << QPointF(-10, -5) << QPointF(-10, 5);
    QPolygonF rhombus;
    rhombus << QPointF(0, -5) << QPointF(5, 0) << QPointF(0, 5) << QPointF(-5, 0);

    std::vector<QGraphicsPolygonItem*> arrows;
    std::vector<QGraphicsPolygonItem*> rhombuses;

    const auto& roadName = Operate->get_road();
    size_t n = Operate->get_numberOfVertices();
    const auto& caps = Operate->get_caps();
    const auto& nodeList = Operate->get_nodeList();

    for(size_t i = 0; i < n; i++)
    {
        QPointF curr = nodes[i]->get_pos();

        for(size_t j = 0; j < n; j++)
        {
            if (node_matrix[i][j] == 0)
                continue;

            QPointF next = nodes[j]->get_pos();

            if (node_matrix[i][j] == node_matrix[j][i])
            {
                double angle = atan2(next.y() - curr.y(), next.x() - curr.x());
                QPointF midpoint = (next + curr) / 2;
                QTransform transform;
                transform.translate(midpoint.x(), midpoint.y());
                transform.rotateRadians(angle);

                QGraphicsPolygonItem* rhombusItem = new QGraphicsPolygonItem(rhombus);
                rhombusItem->setPen(QPen(Qt::black, 3));
                rhombusItem->setBrush(QBrush(Qt::black));
                rhombusItem->setTransform(transform);
                rhombuses.push_back(rhombusItem);

                node_matrix[j][i] = 0;
            }
            else
            {
                double angle = atan2(next.y() - curr.y(), next.x() - curr.x());
                QPointF midpoint = (next + curr) / 2;
                QTransform transform;
                transform.translate(midpoint.x(), midpoint.y());
                transform.rotateRadians(angle);

                QGraphicsPolygonItem* arrowItem = new QGraphicsPolygonItem(arrow);
                arrowItem->setBrush(QBrush(Qt::black));
                arrowItem->setTransform(transform);
                arrows.push_back(arrowItem);
            }

            QString str1;
            const double& x = node_matrix[i][j];
            if (x < 1)
                str1 = "Distance: " + QString::number(x * 1000, 'd', 2) + 'm';
            else
                str1 = "Distance: " + QString::number(x, 'f', 2) + "km";
            str1 += "\nCapacity: " + QString::number(caps[i][j]);
            str1 += "\n\n" + QString::fromStdString(nodeList[i].name) +
                    " -> " + QString::fromStdString(nodeList[j].name);

            QPainterPath roadPath;
            roadPath.moveTo(curr);
            roadPath.lineTo(next);
            customPath* roadP = new customPath(roadPath, str1,"R. " + QString::fromStdString(roadName[i][j]));
            roadP->setPen(QPen(Qt::black, 3));
            scene->addItem(roadP);
        }
    }

    for(const auto& arrowItem : arrows)
        scene->addItem(arrowItem);

    for(const auto& rhombusItem : rhombuses)
        scene->addItem(rhombusItem);
}


void MainWindow::addNodesToScene() {
    qreal minLat = 10.7481;
    qreal maxLat = 10.8711;
    qreal minLong = 106.5780;
    qreal maxLong = 106.7212;

    qreal latScaleFactor = scene->height() / (maxLat - minLat);
    qreal longScaleFactor = scene->width() / (maxLong - minLong);

    const auto& nodelist = Operate->get_nodeList();
    size_t listSize = nodelist.size();

    for (const node& node : nodelist) {
        qreal scaledX = (node.lon - minLong) * longScaleFactor;
        qreal scaledY = scene->height() - (node.lat - minLat) * latScaleFactor;
        QString name = QString::fromStdString(node.name);

        QGraphicsEllipseItem *innerCirc;
        customEllipse *outCir;
        QString label;

        if (node.roundabout)
            label = "Roundabout";
        else if (node.name.find("Node") == std::string::npos)
            label = "Place";
        else if (node.deg <= 2)
            label = "Road node";
        else if (node.name.find("Cang hang khong") != std::string::npos)
            label = "Airport";
        else label = "Intersection";


        if (label == "Roundabout")
        {
            QColor color(214,6,34);
            innerCirc = new QGraphicsEllipseItem(scaledX - 4, scaledY - 4, 8, 8);
            innerCirc->setBrush(color);
            innerCirc->setPen(QPen(color, 2));
            outCir = new customEllipse(scaledX - 8, scaledY - 8, 16, 16, name, label, node.roundabout, scaledX, scaledY);
            outCir->setPen(QPen(color, 2));
        }
        else if (label == "Road node" || label == "Intersection")
        {
            innerCirc = new QGraphicsEllipseItem(scaledX - 1, scaledY - 1, 2, 2);
            innerCirc->setBrush(Qt::red);
            innerCirc->setPen(QPen(Qt::red, 2));
            outCir = new customEllipse(scaledX - 4, scaledY - 4, 8, 8, name, label, node.roundabout, scaledX, scaledY);
            outCir->setPen(QPen(Qt::red, 2));
        }
        else
        {
            QColor color(13,85,255);
            innerCirc = new QGraphicsEllipseItem(scaledX - 4, scaledY - 4, 8, 8);
            innerCirc->setBrush(color);
            innerCirc->setPen(QPen(color, 2));
            outCir = new customEllipse(scaledX - 8, scaledY - 8, 16, 16, name, label, node.roundabout, scaledX, scaledY);
            outCir->setPen(QPen(color, 2));
        }
        scene->addItem(innerCirc);
        scene->addItem(outCir);
        scene->reserve(listSize);
        scene->addNodes(outCir);
    }
}

void MainWindow::removeProperties()
{
    paths.clear();
    pathStr.clear();
    weight.clear();
    label.clear();
    index = 0;
    if (startNode)
    {
        scene->removeItem(startNode);
        delete startNode;
        startNode = nullptr;
    }
    if (endNode)
    {
        scene->removeItem(endNode);
        delete endNode;
        endNode = nullptr;
    }
}

bool MainWindow::checkInput(int source, int end)
{
    if (source == -1)
    {
        if (end == -1)
        {
            QMessageBox::warning(this, "Input Error", "Both of start location and desination are invalid!");
            ui->chooseStart->setFocus();
        }
        else
        {
            QMessageBox::warning(this, "Input Error", "Your start location is invalid!");
            ui->chooseStart->setFocus();
        }
    }
    else if (end == -1)
    {
        QMessageBox::warning(this, "Input Error", "Your destination is invalid!");
        ui->chooseDes->setFocus();
    }
    else if (source == end)
        QMessageBox::information(this, "Wonderful!", "You have reached the destination!");
    else return true;

    return false;
}


void MainWindow::on_resetbutton_clicked()
{
    currentMode = "";
    ui->chooseDes->setText("");
    ui->chooseStart->setText("");
    ui->chooseDes->setPlaceholderText("  Choose destination");
    ui->chooseStart->setPlaceholderText("  Choose start location");
    ui->outText->setText("");
    ui->startlabel->setText("Please select start location.");
    ui->endlabel->setText("Please select destination.");
    ui->startlabel->setStyleSheet("color: solid black");
    ui->endlabel->setStyleSheet("color: solid black");
    graphMode = 1;


    ui->labelPath->setText("Tap above to see the result");
    removePath();
    removeProperties();
    ui->prevbutton->setEnabled(false);
    ui->nextbutton->setEnabled(false);
    ui->switchbutton->setEnabled(false);
    ui->flowbutton->setEnabled(false);
    ui->spbutton->setEnabled(false);
    ui->nextbutton->hide();
    ui->prevbutton->hide();
    ui->buttonStart->setEnabled(true);
    ui->buttonEnd->setEnabled(true);
}

void MainWindow::on_switchbutton_clicked()
{
    auto temp = ui->chooseStart->text();
    ui->chooseStart->setText(ui->chooseDes->text());
    ui->chooseDes->setText(temp);
}


MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_spbutton_clicked()
{
    if (checkInput(source, end))
    {
        removeProperties();
        ui->nextbutton->setEnabled(true);
        ui->prevbutton->setEnabled(false);

        ui->nextbutton->show();
        ui->prevbutton->show();
        addStartEnd(source, end);
        index = 0;

        currentMode = "Distance";

        paths = Operate->get_KSP(source, end, 5);
        auto pstr = Operate->printPath(paths);
        weight = Operate->get_weight((paths));

        drawPath(paths, weight, currentMode);

        QString outstr = "";
        this->pathStr.reserve(6);

        for(const std::string& str : pstr)
        {
            auto temp = QString::fromStdString(str);
            pathStr.push_back(temp);
            outstr += temp + "\n\n";
        }
        pathStr.insert(pathStr.begin(), outstr);
        ui->outText->setText(outstr);

        label.reserve(paths.size() + 1);
        label.emplace_back("Top " + QString::number(paths.size()) + " shortest paths");
        ui->labelPath->setText(label[0]);

        for(size_t i = 1; i <= paths.size(); i++)
        {
            QString labelStr = QString::number(i);
            int digit = i % 10;
            if (digit == 1)
                labelStr += "st";
            else if (digit == 2)
                labelStr += "nd";
            else if (digit == 3)
                labelStr += "rd";
            else labelStr += "th";

            QString num;
            if (weight[i - 1] < 1)
                num = QString::number(static_cast<int>(weight[i - 1] * 1000)) + "m";
            else num = QString::number(weight[i - 1], 'f', 2) + "km";
            labelStr += " shortest path\nTotal distance: " + num;
            label.push_back(labelStr);
        }
    }
}


void MainWindow::on_chooseStart_returnPressed()
{
    if (Operate->nodeToIndex(ui->chooseStart->text().toStdString()) == -1)
        QMessageBox::warning(this, "Input Error", "Your start location is invalid!");
    else
        ui->chooseDes->setFocus();
}


void MainWindow::on_chooseDes_returnPressed()
{
    if (Operate->nodeToIndex(ui->chooseDes->text().toStdString()) == -1)
    {
        QMessageBox::warning(this, "Input Error", "Your destination is invalid!");
        ui->chooseDes->setFocus();
    }
    else
        ui->spbutton->setFocus();
}


void MainWindow::on_flowbutton_clicked()
{
    if (checkInput(source, end))
    {
        removeProperties();
        ui->nextbutton->setEnabled(true);
        ui->prevbutton->setEnabled(false);

        ui->nextbutton->show();
        ui->prevbutton->show();
        addStartEnd(source, end);
        index = 0;

        const auto& maxFlow = Operate->get_maxFlow(source, end);
        currentMode = "Capacity";

        paths = std::get<1>(maxFlow);
        weight = std::get<2>(maxFlow);

        auto pstr = Operate->printFlow(maxFlow);
        drawPath(paths, weight, currentMode);

        QString outstr = "";
        this->pathStr.reserve(paths.size() + 1);

        for(const std::string& str : pstr)
        {
            auto temp = QString::fromStdString(str);
            pathStr.push_back(temp);
            outstr += temp + "\n\n";
        }
        pathStr.insert(pathStr.begin(), outstr);
        ui->outText->setText(outstr);


        label.reserve(paths.size() + 1);
        label.emplace_back("Total number of paths flow: " + QString::number(paths.size())
                           + "\nMaximum flow: " + QString::number(std::get<0>(maxFlow)));
        ui->labelPath->setText(label[0]);

        for(size_t i = 1; i <= paths.size(); i++)
        {
            QString labelStr = QString::number(i);
            int digit = i % 10;
            if (digit == 1)
                labelStr += "st";
            else if (digit == 2)
                labelStr += "nd";
            else if (digit == 3)
                labelStr += "rd";
            else labelStr += "th";
            labelStr += " path\nPath capacity: " + QString::number(weight[i - 1]);
            label.push_back(labelStr);
        }

    }
}


void MainWindow::on_buttonStart_clicked()
{
    scene->set_startButton();
    ui->buttonEnd->setEnabled(false);
}

void MainWindow::on_buttonEnd_clicked()
{
    scene->set_endButton();
    ui->buttonEnd->setFocus();
    ui->buttonStart->setEnabled(false);
}

void MainWindow::set_startText(const QString& text)
{
    ui->chooseStart->setText("  " + text);
    ui->buttonEnd->setEnabled(1);
    ui->buttonEnd->click();
    ui->buttonStart->setEnabled(1);
}

void MainWindow::set_endText(const QString& text)
{
    ui->chooseDes->setText("  " + text);
    ui->spbutton->setFocus();
    ui->buttonEnd->setEnabled(1);
    ui->buttonStart->setEnabled(1);
}

void MainWindow::onEnterPressed(customButton* button)
{
    button->pressed();
}


void MainWindow::on_nextbutton_clicked()
{
    if (currentMode == "" || ui->outText->document()->isEmpty())
        return;
    index++;
    ui->outText->setText(pathStr[index]);
    ui->labelPath->setText(label[index]);
    ui->prevbutton->setEnabled(true);

    drawPath(paths[index - 1], weight[index - 1], currentMode);

    if (index == paths.size())
        ui->nextbutton->setEnabled(false);
}

void MainWindow::on_prevbutton_clicked()
{
    if (currentMode == "" || ui->outText->document()->isEmpty())
        return;

    index--;
    ui->outText->setText(pathStr[index]);
    ui->labelPath->setText(label[index]);
    ui->nextbutton->setEnabled(true);

    if (index == 0)
    {
        drawPath(paths, weight, currentMode);
        ui->prevbutton->setEnabled(false);
    }
    else
        drawPath(paths[index - 1], weight[index - 1], currentMode);
}


void MainWindow::on_graph_clicked()
{
    if (graphMode)
    {
        drawMatrix(Operate->get_matrix());
        graphMode = 0;
    }
    else
    {
        if (currentMode == "Distance")
            ui->spbutton->click();
        else if (currentMode == "Capacity")
            ui->flowbutton->click();
        else removePath();
        graphMode = 1;
    }
}

