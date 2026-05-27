#include <iostream>
#include <vector>
#include <climits>
#include <queue>
#include <ctime>
#include <algorithm>
#include <cmath>
#include <array>
#define CPPHTTPLIB_VERSION_MAJOR 0
#include "httplib.h"
#include "json.hpp"
#include<optional>

using namespace std;
using json = nlohmann::json;
struct Edge {
    int destination;
    int weight;
    int traffic;
};
class Graph {
    int vertices;
    vector<vector<Edge>> adjlist;
public:
    Graph(int V = 0) {
        vertices = V;
        adjlist.resize(vertices);
    }
    void setVertices(int V) {
        vertices = V;
        adjlist.clear();
        adjlist.resize(vertices);
    }
    void addEdge(int u, int v, int w, int t) {
        adjlist[u].push_back({v, w, t});
        adjlist[v].push_back({u, w, t});
    }
    void buildFromJSON(vector<array<int,3>>edges) {
        for (auto &e : edges)
            addEdge(e[0], e[1], e[2], 0);
    }
    void simulateTraffic() {
        for (int u = 0; u < vertices; u++) {
            for (auto &e : adjlist[u]) {
                e.traffic = rand() % 10 + 1;
            }
        }
    }
    vector<Edge>& getAdjList(int node) {
        return adjlist[node];
    }
    pair<int, vector<int>>dijkstra(int src, int dest) {
        vector<int> dist(vertices, INT_MAX);
        vector<int> parent(vertices, -1);
        priority_queue<pair<int,int>, vector<pair<int,int>>, greater<pair<int,int>>> pq;
        dist[src] = 0;
        pq.push({0, src});
        while (!pq.empty()) {
            auto [cd, node] = pq.top();
            pq.pop();
            if (cd != dist[node]) continue;
            for (auto &e : adjlist[node]) {
                int cost = e.weight + e.traffic;
                if (dist[node] + cost < dist[e.destination]) {
                    dist[e.destination] = dist[node] + cost;
                    parent[e.destination] = node;
                    pq.push({dist[e.destination], e.destination});
                }
            }
        }
        vector<int> path;
        for (int v = dest; v != -1; v = parent[v])
            path.push_back(v);
        reverse(path.begin(), path.end());
        return {dist[dest], path};
    }
    int heuristic(int u, int v) {
        return abs(u - v);
    }
    pair<int, vector<int>> aStar(int src, int dest) {
        vector<int> g(vertices, INT_MAX);
        vector<int> f(vertices, INT_MAX);
        vector<int> parent(vertices, -1);
        priority_queue<pair<int,int>, vector<pair<int,int>>, greater<pair<int,int>>> pq;
        g[src] = 0;
        f[src] = heuristic(src, dest);
        pq.push({f[src], src});
        while (!pq.empty()) {
            int node = pq.top().second;
            pq.pop();
            if (node == dest) break;
            for (auto &e : adjlist[node]) {
                int tentative = g[node] + e.weight + e.traffic;
                if (tentative < g[e.destination]) {
                    g[e.destination] = tentative;
                    f[e.destination] = tentative + heuristic(e.destination, dest);
                    parent[e.destination] = node;
                    pq.push({f[e.destination], e.destination});
                }
            }
        }
        vector<int> path;
        for (int v = dest; v != -1; v = parent[v])
            path.push_back(v);

        reverse(path.begin(), path.end());

        return {g[dest], path};
    }
};
int main() {
    srand(time(0));
    httplib::Server svr;
    svr.Options("/route", [](const httplib::Request&, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        res.set_header("Access-Control-Allow-Methods", "POST, OPTIONS");
    });
    svr.Post("/route", [&](const httplib::Request &req, httplib::Response &res) {
        json body;
        try {
            body = json::parse(req.body);
        } catch (...) {
            res.status = 400;
            res.set_content("Invalid JSON", "text/plain");
            return;
        }
        int vertices = body["vertices"];
        int source = body["source"];
        int destination = body["destination"];
        Graph g(vertices);
        vector<array<int,3>> edges;
        for (auto &e : body["graph"]) {
            edges.push_back({e["u"], e["v"], e["w"]});
        }
        g.buildFromJSON(edges);
        g.simulateTraffic();
        auto d = g.dijkstra(source, destination);
        auto a = g.aStar(source, destination);
        json traffic = json::array();
        for (int i = 0; i < vertices; i++) {
            for (auto &e : g.getAdjList(i)) {
                traffic.push_back({
                    {"from", i},
                    {"to", e.destination},
                    {"weight", e.weight},
                    {"traffic", e.traffic}
                });
            }
        }
        json result = {
            {"traffic", traffic},
            {"Dijkstra_time", d.first},
            {"Dijkstra_path", d.second},
            {"Astar_time", a.first},
            {"Astar_path", a.second}
        };
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content(result.dump(), "application/json");
    });
    cout << "Server running at http://localhost:8080\n";
    svr.listen("0.0.0.0", 8080);
    return 0;
}

//C:\msys64\ucrt64\bin\g++.exe TimeMan.cpp -o traffic.exe -std=c++17 -lws2_32
//.\traffic.exe