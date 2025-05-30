#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <limits>
#include <queue>
#include <stack>
#include <functional>
#include <string>

using namespace std;

template<typename Vertex, typename Distance = double>
class Graph {
public:
    struct Edge {
        Vertex from;
        Vertex to;
        Distance distance;

        Edge() : from(), to(), distance() {}  // ����������� �� ���������
        Edge(const Vertex& f, const Vertex& t, const Distance& d)
            : from(f), to(t), distance(d) {
        }

        bool operator==(const Edge& other) const {
            return from == other.from && to == other.to && distance == other.distance;
        }
    };

private:
    vector<Vertex> _vertices;
    multimap<Vertex, Edge> _edges;

public:
    // ��������-����������-�������� ������
    bool has_vertex(const Vertex& v) const {
        return find(_vertices.begin(), _vertices.end(), v) != _vertices.end();
    }

    bool add_vertex(const Vertex& v) {
        if (has_vertex(v)) {
            return false;
        }
        _vertices.push_back(v);
        return true;
    }

    bool remove_vertex(const Vertex& v) {
        auto it = find(_vertices.begin(), _vertices.end(), v);
        if (it == _vertices.end()) {
            return false;
        }
        _vertices.erase(it);

        // ������� ��� �����, ��������� � ���� ��������
        auto range = _edges.equal_range(v);
        _edges.erase(range.first, range.second);

        // ������� ��� �����, ������� � ���� �������
        for (auto it = _edges.begin(); it != _edges.end(); ) {
            if (it->second.to == v) {
                it = _edges.erase(it);
            }
            else {
                ++it;
            }
        }
        return true;
    }

    vector<Vertex> vertices() const {
        return _vertices;
    }


    void add_edge(const Vertex& from, const Vertex& to, const Distance& d) {
        if (!has_vertex(from) || !has_vertex(to)) {
            throw invalid_argument("One or both vertices don't exist");
        }
        _edges.insert(make_pair(from, Edge(from, to, d)));
    }

    void add_undirected_edge(const Vertex& v1, const Vertex& v2, const Distance& d) {
        if (!has_vertex(v1) || !has_vertex(v2)) {
            throw invalid_argument("One or both vertices don't exist");
        }
        _edges.insert(make_pair(v1, Edge(v1, v2, d)));
        _edges.insert(make_pair(v2, Edge(v2, v1, d))); 
    }


    bool remove_edge(const Vertex& from, const Vertex& to) {
        bool removed = false;
        for (auto it = _edges.begin(); it != _edges.end(); ) {
            if (it->first == from && it->second.to == to) {
                it = _edges.erase(it);
                removed = true;
            }
            else {
                ++it;
            }
        }
        return removed;
    }

    bool remove_edge(const Edge& e) {
        auto range = _edges.equal_range(e.from);
        for (auto it = range.first; it != range.second; ++it) {
            if (it->second == e) {
                _edges.erase(it);
                return true;
            }
        }
        return false;
    }

    bool has_edge(const Vertex& from, const Vertex& to) const {
        auto range = _edges.equal_range(from);
        for (auto it = range.first; it != range.second; ++it) {
            if (it->second.to == to) {
                return true;
            }
        }
        return false;
    }

    bool has_edge(const Edge& e) const {
        auto range = _edges.equal_range(e.from);
        for (auto it = range.first; it != range.second; ++it) {
            if (it->second == e) {
                return true;
            }
        }
        return false;
    }

    // ��������� ���� �����, ��������� �� �������
    vector<Edge> edges(const Vertex& vertex) {
        vector<Edge> result;
        auto range = _edges.equal_range(vertex);
        for (auto it = range.first; it != range.second; ++it) {
            result.push_back(it->second);
        }
        return result;
    }

    size_t order() const {
        return _vertices.size();
    }

    size_t degree(const Vertex& v) const {
        return _edges.count(v);
    }

    // �������� ������� ���������
    bool is_connected() const {
        if (_vertices.empty()) return true;

        // ����� � ������� ��� ��������� �����
        set<Vertex> visited;
        stack<Vertex> stack;
        stack.push(_vertices[0]);
        visited.insert(_vertices[0]);

        while (!stack.empty()) {
            Vertex current = stack.top();
            stack.pop();

            auto range = _edges.equal_range(current);
            for (auto it = range.first; it != range.second; ++it) {
                if (visited.find(it->second.to) == visited.end()) {
                    visited.insert(it->second.to);
                    stack.push(it->second.to);
                }
            }
        }

        if (visited.size() != _vertices.size()) {
            return false;
        }

        // ������� ����������������� ����
        Graph<Vertex, Distance> transposed;
        for (size_t i = 0; i < _vertices.size(); ++i) {
            transposed.add_vertex(_vertices[i]);
        }
        for (auto it = _edges.begin(); it != _edges.end(); ++it) {
            const Edge& edge = it->second;
            transposed.add_edge(edge.to, edge.from, edge.distance);  // ����������� �����
        }

        // ����� � ������� ��� ������������������ �����
        visited.clear();
        stack.push(_vertices[0]);
        visited.insert(_vertices[0]);

        while (!stack.empty()) {
            Vertex current = stack.top();
            stack.pop();

            auto range = transposed._edges.equal_range(current);
            for (auto it = range.first; it != range.second; ++it) {
                if (visited.find(it->second.to) == visited.end()) {
                    visited.insert(it->second.to);
                    stack.push(it->second.to);
                }
            }
        }

        return visited.size() == _vertices.size();
    }

    vector<Edge> shortest_path(const Vertex& from, const Vertex& to) const {
        // �������� �� ������������� ����
        for (auto it = _edges.begin(); it != _edges.end(); ++it) {
            if (it->second.distance < 0) {
                throw runtime_error("Dijkstra's algorithm cannot handle negative edge weights");
            }
        }

        // ������������ ������� ��� �������� ������ � �� ������� ����������
        priority_queue<pair<Distance, Vertex>, vector<pair<Distance, Vertex>>, greater<pair<Distance, Vertex>>> pq;

        // ������ ���������� �� ��������� �������
        map<Vertex, Distance> distances;
        // ������ ���������������� ��� �������������� ����
        map<Vertex, Edge> predecessors;

        // ������������� ����������
        for (const auto& v : _vertices) {
            distances[v] = numeric_limits<Distance>::max();
        }
        distances[from] = Distance();
        pq.push(make_pair(distances[from], from));

        while (!pq.empty()) {
            Vertex u = pq.top().second;
            Distance dist_u = pq.top().first;
            pq.pop();

            // ���� ����������� ���������� ������ ��������, ����������
            if (dist_u > distances[u]) {
                continue;
            }

            // �������� ��� ��������� ����� �� ������� u
            auto range = _edges.equal_range(u);
            for (auto it = range.first; it != range.second; ++it) {
                const Edge& edge = it->second;
                Vertex v = edge.to;
                Distance weight = edge.distance;

                // ���������� �����
                if (distances[v] > distances[u] + weight) {
                    distances[v] = distances[u] + weight;
                    predecessors[v] = edge;
                    pq.push(make_pair(distances[v], v));
                }
            }
        }

        // ���� ���� �� ������
        if (distances[to] == numeric_limits<Distance>::max()) {
            return {};
        }

        // �������������� ����
        vector<Edge> path;
        Vertex current = to;
        while (current != from) {
            auto it = predecessors.find(current);
            if (it == predecessors.end()) {
                return {};
            }
            path.push_back(it->second);
            current = it->second.from;
        }

        reverse(path.begin(), path.end());
        return path;
    }

    vector<Vertex> walk(const Vertex& start_vertex) const {
        vector<Vertex> result;
        if (!has_vertex(start_vertex)) {
            return result;
        }

        set<Vertex> visited;
        stack<Vertex> stack;
        stack.push(start_vertex);
        visited.insert(start_vertex);

        while (!stack.empty()) {
            Vertex current = stack.top();
            stack.pop();
            result.push_back(current);

            auto range = _edges.equal_range(current);
            for (auto it = range.first; it != range.second; ++it) {
                if (visited.find(it->second.to) == visited.end()) {
                    visited.insert(it->second.to);
                    stack.push(it->second.to);
                }
            }
        }

        return result;
    }

    // ��������������� ������� ��� ���������� �������� ���������� (� �������������� ��������)
    Distance average_distance(const Vertex& from) const {
        // �������� �� ������������� ����
        for (auto it = _edges.begin(); it != _edges.end(); ++it) {
            if (it->second.distance < 0) {
                throw runtime_error("Dijkstra's algorithm cannot handle negative edge weights");
            }
        }

        priority_queue<pair<Distance, Vertex>, vector<pair<Distance, Vertex>>, greater<pair<Distance, Vertex>>> pq;
        map<Vertex, Distance> distances;

        for (const auto& v : _vertices) {
            distances[v] = numeric_limits<Distance>::max();
        }
        distances[from] = Distance();
        pq.push(make_pair(distances[from], from));

        while (!pq.empty()) {
            Vertex u = pq.top().second;
            Distance dist_u = pq.top().first;
            pq.pop();

            if (dist_u > distances[u]) {
                continue;
            }

            auto range = _edges.equal_range(u);
            for (auto it = range.first; it != range.second; ++it) {
                const Edge& edge = it->second;
                Vertex v = edge.to;
                Distance weight = edge.distance;

                if (distances[v] > distances[u] + weight) {
                    distances[v] = distances[u] + weight;
                    pq.push(make_pair(distances[v], v));
                }
            }
        }

        Distance sum = Distance();
        size_t count = 0;
        for (auto it = distances.begin(); it != distances.end(); ++it) {
            if (it->second != numeric_limits<Distance>::max()) {
                sum += it->second;
                count++;
            }
        }

        if (count == 0) return Distance();
        return static_cast<Distance>(sum) / count;
    }

    Vertex find_optimal_warehouse() const {
        if (_vertices.empty()) {
            throw runtime_error("Graph is empty");
        }

        Vertex optimal = _vertices[0];
        Distance min_avg = numeric_limits<Distance>::max();

        for (size_t i = 0; i < _vertices.size(); ++i) {
            Distance avg = average_distance(_vertices[i]);
            if (avg < min_avg) {
                min_avg = avg;
                optimal = _vertices[i];
            }
        }

        return optimal;
    }
    double average_edge_length(const Vertex& v) const {
        auto range = _edges.equal_range(v);
        size_t count = 0;
        Distance sum = 0;

        for (auto it = range.first; it != range.second; ++it) {
            sum += it->second.distance;
            count++;
        }

        for (auto it = _edges.begin(); it != _edges.end(); ++it) {
            if (it->second.to == v) {
                sum += it->second.distance;
                count++;
            }
        }

        if (count == 0) return 0.0;
        return static_cast<double>(sum) / count;
    }

    // ������� ��� ���������� ����������� � ������������ ������� ������ �����
    Vertex find_most_remote_clinic() const {
        if (_vertices.empty()) {
            throw runtime_error("���� ����");
        }

        Vertex result = _vertices[0];
        double max_avg = average_edge_length(result);

        for (size_t i = 1; i < _vertices.size(); ++i) {
            double current_avg = average_edge_length(_vertices[i]);
            if (current_avg > max_avg) {
                max_avg = current_avg;
                result = _vertices[i];
            }
        }

        return result;
    }
};

void analyze_samara_clinics() {
    Graph<string> samaraGraph;

    samaraGraph.add_vertex("���������� �1 (��������� �����, 56)");
    samaraGraph.add_vertex("���������� �2 (����-�������, 159)");
    samaraGraph.add_vertex("���������� �3 (������, 150)");
    samaraGraph.add_vertex("���������� �4 (�����-������, 142)");
    samaraGraph.add_vertex("���������� �5 (����������������, 71)");
    samaraGraph.add_vertex("���������� �6 (������, 96�)");
    samaraGraph.add_vertex("���������� �7 (��������, 24)");


    samaraGraph.add_undirected_edge("���������� �1 (��������� �����, 56)",
                       "���������� �7 (��������, 24)", 1.5);
    samaraGraph.add_undirected_edge("���������� �2 (����-�������, 159)",
                       "���������� �4 (�����-������, 142)", 3.2);
    samaraGraph.add_undirected_edge("���������� �3 (������, 150)",
                       "���������� �6 (������, 96�)", 4.1);
    samaraGraph.add_undirected_edge("���������� �4 (�����-������, 142)",
                       "���������� �5 (����������������, 71)", 2.8);
    samaraGraph.add_undirected_edge("���������� �5 (����������������, 71)",
                       "���������� �7 (��������, 24)", 2.3);
    samaraGraph.add_undirected_edge("���������� �6 (������, 96�)",
                       "���������� �2 (����-�������, 159)", 5.0);
    samaraGraph.add_undirected_edge("���������� �1 (��������� �����, 56)",
                       "���������� �4 (�����-������, 142)", 3.5);

    if (!samaraGraph.is_connected()) {
        cout << "������: ���� ������������ ���������! ��������� ������ �����������.\n";
        return;
    }

    //����� ������ ���������� �����������
    string remoteClinic = samaraGraph.find_most_remote_clinic();
    double avgDist = samaraGraph.average_edge_length(remoteClinic);

    cout << "������ ������������ ������:\n";
    cout << "����� ��������� ����������: " << remoteClinic << endl;
    cout << "������� ���������� �� �������: " << avgDist << " ��\n\n";

    cout << "������� ���������� ��� ���� ������������:\n";
    for (const auto& clinic : samaraGraph.vertices()) {
        cout << clinic << ": " 
             << samaraGraph.average_edge_length(clinic) << " ��\n";
    }
}

int main() {
    setlocale(LC_ALL, "Russian");
    analyze_samara_clinics();
    return 0;
}