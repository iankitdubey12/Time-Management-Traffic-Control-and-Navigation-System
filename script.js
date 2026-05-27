async function run() {

    let vertices = document.getElementById("vertices").value;
    let edges = document.getElementById("edges").value;
    let source = document.getElementById("source").value;
    let destination = document.getElementById("destination").value;

    let lines = document.getElementById("graphInput").value.trim().split("\n");

    let graph = [];

    lines.forEach(line => {
        let p = line.trim().split(" ");
        if(p.length === 3){
            graph.push({
                u: +p[0],
                v: +p[1],
                w: +p[2]
            });
        }
    });

    let res = await fetch("http://localhost:8080/route", {
        method: "POST",
        headers: {"Content-Type": "application/json"},
        body: JSON.stringify({
            vertices: +vertices,
            edges: +edges,
            graph: graph,
            source: +source,
            destination: +destination
        })
    });

    let data = await res.json();

    /* 🚦 TRAFFIC DISPLAY */
    let html = "";

    data.traffic.forEach(e => {

        let cls = "low";
        if(e.traffic > 6) cls = "high";
        else if(e.traffic > 3) cls = "medium";

        html += `
            <div class="traffic-item ${cls}">
                ${e.from} → ${e.to}
                (w=${e.weight}, t=${e.traffic})
            </div>
        `;
    });

    document.getElementById("traffic").innerHTML = html;

    /* DIJKSTRA */
    document.getElementById("dijkstra").innerHTML =
        `⏱ Time: ${data.Dijkstra_time}<br>📍 Path: ${data.Dijkstra_path}`;

    /* A* */
    document.getElementById("astar").innerHTML =
        `⏱ Time: ${data.Astar_time}<br>📍 Path: ${data.Astar_path}`;
}