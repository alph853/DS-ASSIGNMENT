#ifndef CLASS_H
#define CLASS_H

#include <QFile>
#include <string>
#include <QDebug>
#include <sstream>
#include <vector>
#include <memory>
#include <cmath>
#include <queue>
#include <limits>
#include <algorithm>
#include <queue>
#include <tuple>
#include <iomanip>
#include <unordered_map>
#include <QString>
#include <QTextStream>

const double PI = 3.141592653;


template <typename T>
using matrix = std::vector<std::vector<T>>;

using path = std::vector<int>;
using flows = const std::tuple<double, matrix<int>, std::vector<double>> &;
using flowfunc = std::tuple<double, matrix<int>, std::vector<double>>;


struct node
{
    double lat, lon;
    std::string name;
    int index;
    int deg;
    bool roundabout = 0;

    node() {}
    node(double lat, double lon) : lat(lat), lon(lon) {}
    node(int index, double lat, double lon, std::string name, bool ra) : lat(lat), lon(lon), name(name), index(index), roundabout(ra) {}
};

class nodeManager
{
public:
    nodeManager(int n) : numberOfVertices(n),
        m_nodeMatrix(n, std::vector<double>(n)),
        m_roadMatrix(n, std::vector<std::string>(n)),
        m_bearing(n, std::vector<double>(n)),
        m_capacity(n, std::vector<double>(n))
    {
        m_nodeList.reserve(n);
    }

    // read files
    void loadRoad(const QString &roadFile);
    void loadNode(const QString &nodeFile);

    // dijkstra and YENKSP
    double weight(const matrix<double> &graph, const path &p) const;
    path dijkstra(const matrix<double> &graph, int source, int end) const;
    matrix<int> topk_shortestPaths(int source, int end, int k) const;

    // Dinic maximum flow
    bool levelBFS(const matrix<double> &graph, int source, int end, std::vector<int> &level);
    std::pair<double, path> sendFlowDFS(matrix<double> &graph, int source, int end, double minCap, const std::vector<int> &level, path dinicpath);
    flowfunc dinicFlow(int source, int end);

    // output direction string
    int nodeToIndex(const std::string &str) const;
    std::vector<node> get_nodeList() const;
    std::string toString(const path &p) const;
    std::vector<std::string> printPath(const matrix<int> &paths) const;
    std::vector<std::string> printFlow(flows maxFlow) const;
    int direction(node A, node B, node C) const;
    matrix<double> get_nodeMatrix() const { return m_nodeMatrix; }
    matrix<std::string> get_road() const { return m_roadMatrix; }
    matrix<double>get_caps() const { return m_capacity; }

private:
    int numberOfVertices;
    std::vector<node> m_nodeList;
    matrix<double> m_nodeMatrix;
    matrix<std::string> m_roadMatrix;
    matrix<double> m_bearing;
    matrix<double> m_capacity;
    std::unordered_map<std::string, int> nodeToIndexMap;
};

class operate
{
public:
    operate(int n) : numberOfVertices(n), m_nodeManager(std::make_unique<nodeManager>(n))
    {
        loadNode("nodeFile.txt");
        loadRoad("roadFile.txt");
    }

    void loadRoad(const QString &roadFile) const;
    void loadNode(const QString &nodeFile) const;

    matrix<int> get_KSP(int source, int end, int k) const;
    std::vector<node> get_nodeList() const;

    int nodeToIndex(const std::string &str) const;

    flowfunc get_maxFlow(int source, int end) const;
    std::vector<std::string> printFlow(flows maxFlow) const;

    std::vector<std::string> printPath(const matrix<int> &paths) const;
    std::vector<double> get_weight(const matrix<int>& p);
    matrix<double> get_matrix() const { return m_nodeManager->get_nodeMatrix(); }
    matrix<std::string> get_road() const { return m_nodeManager->get_road(); }
    size_t get_numberOfVertices() const { return numberOfVertices; }
    matrix<double> get_caps() const { return m_nodeManager->get_caps(); }

private:
    int numberOfVertices;
    std::unique_ptr<nodeManager> m_nodeManager;
};

#endif // CLASS_H
