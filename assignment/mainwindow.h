#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "customgraphics.h"
#include "class.h"
#include <QVBoxLayout>
#include <QCompleter>

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_resetbutton_clicked();
    void on_switchbutton_clicked();
    void on_spbutton_clicked();

    void textStart();
    void textEnd();


    void on_chooseStart_returnPressed();
    void on_chooseDes_returnPressed();


    void on_flowbutton_clicked();

    void on_buttonStart_clicked();
    void on_buttonEnd_clicked();

    void set_startText(const QString& );
    void set_endText(const QString& );

    void onEnterPressed(customButton *);


    void on_nextbutton_clicked();
    void on_prevbutton_clicked();

    void on_graph_clicked();

private:
    int source = -1, end = -1;
    Ui::MainWindow *ui;
    customScene *scene;
    std::unique_ptr<operate> Operate;
    QGraphicsView *view;

    void drawPath(const std::vector<int> &p, double weight, const QString& currentMode);
    void drawPath(const matrix<int>& p, const std::vector<double>& weight, const QString& mode);
    void drawMatrix(const matrix<double> &nodeMatrix);
    void removePath();
    void addStartEnd(int source, int end);
    void addNodesToScene();
    void removeProperties();
    bool checkInput(int source, int end);

    std::vector<QColor> color;
    customEllipse* startNode = nullptr;
    customEllipse* endNode = nullptr;

    matrix<int> paths;
    std::vector<double> weight;
    std::vector<QString> pathStr;
    size_t index = 0;

    std::vector<QString> label;

    QString currentMode = "";
    bool graphMode = 1;
};



#endif // MAINWINDOW_H
