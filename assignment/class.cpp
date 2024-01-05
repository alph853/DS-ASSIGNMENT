#include "class.h"


inline std::string compass[] = {"north", "northeast", "east", "southeast", "south", "southwest", "west", "northwest"};
inline auto inf = std::numeric_limits<double>::infinity();


double toRadians(double deg)
{
    return deg * PI / 180.0;
}

std::pair<double, double> haversineDistanceAndBearing(const node &A, const node &B)
{
    const double R = 6378.0;
    double lat1 = toRadians(A.lat);
    double lon1 = toRadians(A.lon);
    double lat2 = toRadians(B.lat);
    double lon2 = toRadians(B.lon);

    double dlat = lat2 - lat1;
    double dlon = lon2 - lon1;

    double a = std::sin(dlat / 2.0) * std::sin(dlat / 2.0) +
               std::cos(lat1) * std::cos(lat2) * std::sin(dlon / 2.0) * std::sin(dlon / 2.0);
    double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));

    double y = sin(dlon) * cos(lat2);
    double x = cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(dlon);
    double bearing = atan2(y, x);

    bearing = bearing * 180.0 / PI;

    // Normalize
    bearing = fmod((bearing + 360.0), 360.0);

    double distance = R * c;

    return {distance, bearing};
}

double nodeManager::weight(const matrix<double> &graph, const path &p) const
{
    double sum = 0;
    for (size_t i = 0; i < p.size() - 1; i++)
    {
        sum += graph[p[i]][p[i + 1]];
    }
    return sum;
}

// operate
matrix<int> operate::get_KSP(int source, int end, int k) const
{
    return m_nodeManager->topk_shortestPaths(source, end, k);
}

std::vector<node> operate::get_nodeList() const
{
    return m_nodeManager->get_nodeList();
}

void operate::loadRoad(const QString &roadFile) const
{
    m_nodeManager->loadRoad(roadFile);
}

void operate::loadNode(const QString &nodeFile) const
{
    m_nodeManager->loadNode(nodeFile);
}

std::vector<std::string> operate::printPath(const matrix<int> &paths) const
{
    return m_nodeManager->printPath(paths);
}

std::vector<double> operate::get_weight(const matrix<int> &p)
{
    std::vector<double> weights;
    weights.reserve(p.size());

    for(const auto& p_i : p)
        weights.push_back(m_nodeManager->weight(m_nodeManager->get_nodeMatrix(), p_i));
    return weights;
}

// nodeManager
void nodeManager::loadNode(const QString &nodeFile)
{
    QFile file(nodeFile);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Error opening file\n";
        return;
    }

    QTextStream in(&file);

    int index = 0;
    while (!in.atEnd())
    {
        auto line = in.readLine().toStdString();
        char delim;
        double lat, lon;
        std::istringstream is(line);

        std::string token;
        std::string nodeName = "";

        is >> token;
        while (is >> token)
        {
            if (token == ",")
            {
                nodeName.erase(nodeName.length() - 1);
                break;
            }
            nodeName += token;
            nodeName += " ";
        }

        is >> lat >> delim >> lon;

        std::string find = is.str();
        size_t pos = find.rfind("-1");
        bool ra = pos != std::string::npos && pos == find.length() - 2;

        if (nodeName == "0")
            nodeName = "Node " + std::to_string(index + 1);

        std::string name = nodeName;
        for(char &c : name)
            c = std::tolower(c);
        nodeToIndexMap[name] = index;

        m_nodeList.emplace_back(index, lat, lon, nodeName, ra);
        index++;
    }
}

void nodeManager::loadRoad(const QString &roadFile)
{
    QFile file(roadFile);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Error opening file\n";
        return;
    }

    QTextStream in(&file);

    while (!in.atEnd())
    {
        std::string line = in.readLine().toStdString();
        std::istringstream is(line);
        std::string token;
        std::string roadname = "";

        while (is >> token)
        {
            if (token == ",")
            {
                roadname.erase(roadname.length() - 1);
                break;
            }
            roadname += token;
            roadname += std::string(" ");
        }

        int oneWay = 0, n, prev;
        double cap;
        is >> cap;
        if (is >> token && token == "mw")
        {
            cap *= 60;
            is >> oneWay;
        }
        else
        {
            cap *= 40 / 15;
            oneWay = std::stoi(token);
        }

        cap *= 1000;
        if (oneWay != -1)
            prev = oneWay;
        else
        {
            is >> prev;
            cap *= 1.2;
        }
        prev--;

        while (is >> n)
        {
            n--;
            m_roadMatrix[prev][n] = roadname;
            std::pair<double, double> Pair = haversineDistanceAndBearing(m_nodeList[prev], m_nodeList[n]);
            std::tie(m_nodeMatrix[prev][n], m_bearing[prev][n]) = Pair;
            m_capacity[prev][n] = cap;

            if (oneWay != -1)
            {
                m_roadMatrix[n][prev] = roadname;
                m_nodeMatrix[n][prev] = Pair.first;
                m_bearing[n][prev] = fmod((Pair.second + 180.0), 360);
                m_capacity[n][prev] = cap * 1.2;
            }
            prev = n;
        }
    }

    int index = 0;
    for (const auto &row : m_nodeMatrix)
    {
        int count = 0;
        for (auto i : row)
        {
            if (i)
                count++;
        }
        m_nodeList[index++].deg = count;
    }
}

path nodeManager::dijkstra(const matrix<double> &graph, int source, int end) const
{
    using vertex = std::pair<double, int>; // weight, index
    std::priority_queue<vertex, std::vector<vertex>, std::greater<>> pVertices;

    size_t n = graph.size();

    std::vector<double> totalWeight(n, inf);
    path prev(n, -1);

    pVertices.push({0.0, source});
    totalWeight[source] = 0.0;

    while (!pVertices.empty())
    {
        auto [distance, current] = pVertices.top();
        pVertices.pop();

        if (current == end)
        {
            path p;
            p.reserve(n);
            while (current != -1)
            {
                p.push_back(current);
                current = prev[current];
            }
            std::reverse(p.begin(), p.end());
            return p;
        }

        for (size_t next = 0; next < n; next++)
        {
            if (graph[current][next])
            {
                double newdis = graph[current][next] + totalWeight[current];
                if (newdis < totalWeight[next])
                {
                    totalWeight[next] = newdis;
                    prev[next] = current;
                    pVertices.push({newdis, next});
                }
            }
        }
    }
    return {};
}

matrix<int> nodeManager::topk_shortestPaths(int source, int end, int k) const
{
    size_t n = numberOfVertices;
    auto graph = m_nodeMatrix;

    auto shortestPath = dijkstra(graph, source, end);

    if (shortestPath.empty())
        return {};

    matrix<int> kShortest;
    kShortest.reserve(k);
    kShortest.push_back(shortestPath);
    matrix<int> tempPath;

    for (int i = 1; i < k; i++)
    {
        size_t lengthSP = kShortest[i - 1].size();
        auto &shortestPath = kShortest[i - 1];
        for (size_t j = 0; j < lengthSP - 1; j++)
        {
            int spurNode = shortestPath[j];
            path rootPath(shortestPath.begin(), shortestPath.begin() + j + 1);

            std::vector<std::tuple<int, int, double>> backupEdge;

            for (const auto &p : kShortest)
            {
                if (rootPath.size() > p.size())
                    continue;
                bool flag = 0;
                for (size_t m = 0; m <= j; m++)
                {
                    if (p[m] != rootPath[m])
                    {
                        flag = 1;
                        break;
                    }
                }
                if (!flag)
                {
                    int u = p[j];
                    int v = p[j + 1];
                    if (graph[u][v] == inf)
                        continue;
                    backupEdge.emplace_back(u, v, graph[u][v]);
                    graph[u][v] = inf;
                }
            }

            matrix<double> backupNodes(n, std::vector<double>(n));
            for (auto node : rootPath)
            {
                if (node == spurNode)
                    break;
                for (size_t m = 0; m < n; m++)
                {
                    if (graph[node][m] == inf)
                        continue;
                    backupNodes[node][m] = graph[node][m];
                    graph[node][m] = inf;
                }
            }

            auto spurPath = dijkstra(graph, spurNode, end);

            for (auto node : rootPath)
            {
                if (node == spurNode)
                    break;
                for (size_t m = 0; m < n; m++)
                {
                    graph[node][m] = backupNodes[node][m];
                }
            }

            for (const auto &e : backupEdge)
            {
                auto &[u, v, x] = e;
                graph[u][v] = x;
            }

            if (spurPath.size() <= 1)
                continue;

            auto totalPath = rootPath;
            for (size_t m = 1; m < spurPath.size(); m++)
            {
                totalPath.push_back(spurPath[m]);
            }

            bool flag = 0;
            for (const auto &p : tempPath)
            {
                if (totalPath == p)
                    flag = 1;
            }
            if (!flag)
                tempPath.push_back(totalPath);
        }

        if (tempPath.empty())
            break;

        std::sort(tempPath.begin(), tempPath.end(), [&](const path &p1, const path &p2)
                  { return weight(graph, p1) < weight(graph, p2); });

        kShortest.push_back(tempPath[0]);
        tempPath.erase(tempPath.begin());
    }
    return kShortest;
}

bool nodeManager::levelBFS(const matrix<double> &graph, int source, int end, std::vector<int> &level)
{
    int n = graph.size();

    for (int &i : level)
        i = -1;

    level[source] = 0;
    std::queue<int> qVertex;
    qVertex.push(source);

    while (!qVertex.empty())
    {
        int u = qVertex.front();
        qVertex.pop();

        for (int v = 0; v < n; v++)
        {
            if (graph[u][v] > 0 && level[v] == -1)
            {
                level[v] = level[u] + 1;
                qVertex.push(v);
            }
        }
    }
    return level[end] != -1;
}

std::pair<double, path> nodeManager::sendFlowDFS(matrix<double> &graph, int u, int end, double minCap, const std::vector<int> &level, path dinicpath)
{
    dinicpath.push_back(u);
    if (u == end)
        return {minCap, dinicpath};
    int n = graph.size();

    for (int v = 0; v < n; v++)
    {
        if (graph[u][v] > 0 && level[v] == level[u] + 1)
        {
            auto [bottleNeck, newPath] = sendFlowDFS(graph, v, end, std::min(minCap, graph[u][v]), level, dinicpath);
            if (bottleNeck > 0)
            {
                graph[u][v] -= bottleNeck;
                graph[v][u] -= bottleNeck;
                return {bottleNeck, newPath};
            }
        }
    }
    return {0, {}};
}

flowfunc nodeManager::dinicFlow(int source, int end)
{
    double flow = 0.0;
    std::vector<int> level(numberOfVertices);
    auto graph = m_capacity;
    matrix<int> paths;
    std::vector<double> caps;
    // std::vector<int> level(6);
    // auto graph = testGraph;

    while (levelBFS(graph, source, end, level))
    {
        auto [blockingFlow, path] = sendFlowDFS(graph, source, end, inf, level, {});
        while (blockingFlow > 0)
        {
            flow += blockingFlow;
            paths.push_back(path);
            caps.push_back(blockingFlow);
            std::tie(blockingFlow, path) = sendFlowDFS(graph, source, end, inf, level, {});
        }
    }
    return {flow, paths, caps};
}

std::vector<std::string> nodeManager::printPath(const matrix<int> &paths) const
{
    std::vector<std::string> result;
    result.reserve(paths.size());

    int i = 1;
    for (const auto &p : paths)
    {
        double totalDistance = 0;
        double distance = 0;
        size_t plen = p.size();

        int curr = p[0];
        std::string outstr = "Currently at ";
        std::string name = m_nodeList[curr].name;
        if (name == "0")
            name = "Node " + std::to_string(m_nodeList[p[0]].index + 1);
        outstr += name + "\n\n";
        outstr += "From " + name + ",";

        bool onBridge = 0;

        for (size_t i = 1; i < plen; i++)
        {
            int next = p[i];
            distance += m_nodeMatrix[curr][next];

            std::string currentRoad = m_roadMatrix[curr][next];

            if (i == plen - 1)
            {
                std::ostringstream stream;
                std::string outDis;
                totalDistance += distance;
                if (distance < 1)
                {
                    distance *= 1000;
                    outDis = std::to_string(int(round(distance))) + "m";
                }
                else
                {
                    stream << std::fixed << std::setprecision(2) << distance;
                    outDis = stream.str() + "km";
                }
                if (outstr.back() == ',')
                    outstr += " head " + compass[int(m_bearing[curr][next] / 360 * 8)] + " on R.\"" + currentRoad;
                else
                    outstr += "Continue on R.\"" + currentRoad;
                outstr += "\" and you'll get to the destination!\n--> Distance = " + outDis + "\n";
                break;
            }

            int turn = p[i + 1];
            std::string nextRoad = m_roadMatrix[next][turn];
            bool roundAbout = m_nodeList[next].roundabout;

            if (!roundAbout)
            {
                if (currentRoad == nextRoad)
                {
                    curr = next;
                    continue;
                }
            }

            int dir = direction(m_nodeList[curr], m_nodeList[next], m_nodeList[turn]);

            if (outstr.back() == ',')
                outstr += " head " + compass[int(m_bearing[curr][next] / 360 * 8)] + " on R.\"" + currentRoad + "\"";
            else if (onBridge)
            {
                outstr += "Continue on Bridge \"" + currentRoad + "\"";
                onBridge = 0;
            }
            else
                outstr += "Stay on R.\"" + currentRoad + "\"";

            if (m_nodeList[next].name.find("Node") == std::string::npos)
                outstr += " to reach \"" + m_nodeList[next].name + "\"";
            else
                outstr += " to reach " + m_nodeList[next].name;

            std::ostringstream stream;
            std::string outDis;
            totalDistance += distance;
            if (distance < 1)
            {
                distance *= 1000;
                outDis = std::to_string(int(round(distance))) + "m";
            }
            else
            {
                stream << std::fixed << std::setprecision(2) << distance;
                outDis = stream.str() + "km";
            }
            outstr += std::string("\n--> Distance = ") + outDis;
            distance = 0;

            if (nextRoad.find("Cau") != std::string::npos)
            {
                onBridge = 1;
                outstr += "\nThen head onto \"" + nextRoad + "\"\n\n";
            }
            else if (roundAbout)
                outstr += "\nThen exit the roundabout onto \"" + nextRoad + "\"\n\n";
            else
            {
                switch (dir)
                {
                case 0:
                    outstr += "\nThen continue straight ahead onto R.\"" + nextRoad + "\"\n\n";
                    break;
                case 1:
                    outstr += "\nThen turn right onto R.\"" + nextRoad + "\"\n\n";
                    break;
                case 2:
                    outstr += "\nThen turn left onto R.\"" + nextRoad + "\"\n\n";
                    break;
                case 3:
                    outstr += "\nThen slight right onto R.\"" + nextRoad + "\"\n\n";
                    break;
                case 4:
                    outstr += "\nThen slight left onto R.\"" + nextRoad + "\"\n\n";
                    break;
                }
            }
            curr = next;
        }

        std::stringstream ss;
        for (const auto &i : p)
            ss << i + 1 << ' ';

        auto str = ss.str();
        str[str.size() - 1] = '\n';

        std::ostringstream stream;
        std::string str1;
        if (totalDistance < 1)
        {
            totalDistance *= 1000;
            str1 = std::to_string(static_cast<int>(totalDistance)) + "m\n";
        }
        else
        {
            stream << std::fixed << std::setprecision(2) << totalDistance;
            str1 = stream.str() + "km\n";
        }

        result.push_back("Path " + std::to_string(i++) + ": " + str + std::string(43, '-') + '\n' + "Total Distance: " + str1 + std::string(43, '-') + '\n' + outstr + '\n');
    }
    return result;
}

double dotProduct(node n1, node n2)
{
    return n1.lat * n2.lat + n1.lon * n2.lon;
}

// Function to calculate the magnitude of a vector
double magnitude(node n)
{
    return sqrt(n.lat * n.lat + n.lon * n.lon);
}

int nodeManager::direction(node A, node B, node C) const
{
    node vectorAB = {B.lat - A.lat, B.lon - A.lon};
    node vectorBC = {C.lat - B.lat, C.lon - B.lon};

    double magnitudeAB = magnitude(vectorAB);
    double magnitudeBC = magnitude(vectorBC);

    node unitAB = {vectorAB.lat / magnitudeAB, vectorAB.lon / magnitudeAB};
    node unitBC = {vectorBC.lat / magnitudeBC, vectorBC.lon / magnitudeBC};

    double dot = dotProduct(unitAB, unitBC);
    double angle = acos(dot) * (180.0 / PI);

    if (angle <= 10 || fabs(angle - 180.0) <= 10)
    {
        return 0;
    }

    // Calculate cross product
    double crossProduct = vectorAB.lat * vectorBC.lon - vectorAB.lon * vectorBC.lat;

    // right
    if (crossProduct > 0)
    {
        // slight right
        if (angle <= 45 || fabs(angle - 180.0) <= 45)
            return 3;
        return 1;
    } // left
    else if (crossProduct < 0)
    {
        // slight left
        if (angle <= 45 || fabs(angle - 180.0) <= 45)
            return 4;
        return 2;
    } // straight
    else
    {
        return 0;
    }
}

flowfunc operate::get_maxFlow(int source, int end) const
{
    return m_nodeManager->dinicFlow(source, end);
}

std::vector<std::string> operate::printFlow(flows maxFlow) const
{
    return m_nodeManager->printFlow(maxFlow);
}

std::string nodeManager::toString(const path &p) const
{
    std::stringstream ss;
    for (const auto &i : p)
        ss << i + 1 << ' ';

    auto str = ss.str();
    str[str.size() - 1] = '\n';
    return str;
}

std::vector<std::string> nodeManager::printFlow(flows maxFlow) const
{
    const auto &paths = std::get<1>(maxFlow);
    const auto &caps = std::get<2>(maxFlow);

    std::vector<std::string> result;

    result.reserve(paths.size() + 1);

    for (size_t i = 0; i < paths.size(); i++)
    {
        std::string outstr = "";
        auto p = paths[i];
        size_t plen = p.size();

        std::stringstream ss;
        for (const auto &x : p)
            ss << x + 1 << ' ';

        auto str = ss.str();
        str[str.size() - 1] = '\n';
        outstr += "Path " + std::to_string(i + 1) + ": " + str + std::string(43, '-') + "\n" + "Capacity: " + std::to_string(static_cast<int>(caps[i])) + '\n' + std::string(43, '-') + "\n";

        int curr = p[0];
        outstr += "Currently at ";
        std::string name = m_nodeList[curr].name;
        if (name == "0")
            name = "Node " + std::to_string(m_nodeList[p[0]].index + 1);
        outstr += name + "\n\n";
        outstr += "From " + name + ",";

        bool onBridge = 0;

        for (size_t i = 1; i < plen; i++)
        {
            int next = p[i];
            std::string currentRoad = m_roadMatrix[curr][next];

            if (i == plen - 1)
            {
                if (outstr.back() == ',')
                    outstr += " head " + compass[int(m_bearing[curr][next] / 360 * 8)] + " on R.\"" + currentRoad;
                else
                    outstr += "Continue on R.\"" + currentRoad;
                outstr += "\" and you'll get to the destination!\n\n";
                break;
            }

            int turn = p[i + 1];
            std::string nextRoad = m_roadMatrix[next][turn];
            bool roundAbout = m_nodeList[next].roundabout;

            if (!roundAbout)
            {
                if (currentRoad == nextRoad)
                {
                    curr = next;
                    continue;
                }
            }

            int dir = direction(m_nodeList[curr], m_nodeList[next], m_nodeList[turn]);

            if (outstr.back() == ',')
                outstr += " head " + compass[int(m_bearing[curr][next] / 360 * 8)] + " on R.\"" + currentRoad + "\"";
            else if (onBridge)
            {
                outstr += "Continue on Bridge \"" + currentRoad + "\"";
                onBridge = 0;
            }
            else
                outstr += "Stay on R.\"" + currentRoad + "\"";

            if (m_nodeList[next].name.find("Node") == std::string::npos)
                outstr += " to reach \"" + m_nodeList[next].name + "\"";
            else
                outstr += " to reach " + m_nodeList[next].name;

            if (nextRoad.find("Cau") != std::string::npos)
            {
                onBridge = 1;
                outstr += "\nThen head onto \"" + nextRoad + "\"\n\n";
            }
            else if (roundAbout)
                outstr += "\nThen exit the roundabout onto \"" + nextRoad + "\"\n\n";
            else
            {
                switch (dir)
                {
                case 0:
                    outstr += "\nThen continue straight ahead onto R.\"" + nextRoad + "\"\n\n";
                    break;
                case 1:
                    outstr += "\nThen turn right onto R.\"" + nextRoad + "\"\n\n";
                    break;
                case 2:
                    outstr += "\nThen turn left onto R.\"" + nextRoad + "\"\n\n";
                    break;
                case 3:
                    outstr += "\nThen slight right onto R.\"" + nextRoad + "\"\n\n";
                    break;
                case 4:
                    outstr += "\nThen slight left onto R.\"" + nextRoad + "\"\n\n";
                    break;
                }
            }
            curr = next;
        }
        result.push_back(outstr);
    }
    return result;
}

int operate::nodeToIndex(const std::string &str) const
{
    return m_nodeManager->nodeToIndex(str);
}

int nodeManager::nodeToIndex(const std::string &str) const
{    
    size_t start = 0;
    size_t end = str.length();
    while (start < end && str[start] == ' ')
        ++start;

    while (end > start && str[end - 1] == ' ')
        --end;

    std::string outstr = str.substr(start, end - start);
    for(char &c : outstr)
        c = std::tolower(c);

    if (nodeToIndexMap.find(outstr) != nodeToIndexMap.end())
        return nodeToIndexMap.at(outstr);
    return -1;
}

std::vector<node> nodeManager::get_nodeList() const
{
    return m_nodeList;
}
