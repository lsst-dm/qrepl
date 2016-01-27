var bh  = 30;     // box height
var bw  = 75;     // box width
var br  = 7;      // box corner radius
var bs  = 2;      // box stroke width
var bm  = 10;     // box spacing margin
var km  = 5;      // extra spacing between nodes and keys
var bfs = "13px"; // box font size
var td  = 300;    // trans. duration

// Get a unique ID for this tab.  Used to suffix XHR requests to avoid e.g.
// cache locking if loaded in more than one tab in Chrome simultaneously...
var tabId = function() {
    var d = performance.now();
    return 'xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx'.replace(/[xy]/g, function(c) {
        var r = (d + Math.random()*16)%16 | 0;
        d = Math.floor(d/16);
        return (c=='x' ? r : (r&0x3|0x8)).toString(16);
    });
}();

var topg = d3.select("svg").append("g").attr("transform", "translate(" + bm + "," + bm + ")");

d3.select("svg").call(d3.behavior.zoom().on("zoom", function () {
    topg.attr("transform", "translate(" + d3.event.translate + ")" + " scale(" + d3.event.scale + ")")
}));

function addNodes(n) {
	for(var i=0; i<n; ++i) {
		d3.json("/api/v1/nodes").post();
	}
}

function remNodes(n) {
	var data = topg.selectAll(".node").data();
	var start = data.length-1;
	var end = Math.max(start-n, -1);
	for(var i=start; i>end; --i) {
		var j = Math.floor(Math.random() * (i+1));
		d3.json("/api/v1/nodes/" + data[j].id).send("DELETE");
		data[j] = data[i];
	}
}

d3.select("#add1node").on("click", function() { addNodes(1); });
d3.select("#add5nodes").on("click", function() { addNodes(5) });
d3.select("#rem5nodes").on("click", function() { remNodes(5); });
d3.select("#rem1node").on("click", function() { remNodes(1); });
d3.select("#add1key").on("click", function() {});
d3.select("#add5keys").on("click", function() {});
d3.select("#rem5keys").on("click", function() {});
d3.select("#rem1key").on("click", function() {});

function bt(x, y) {
    return "translate(" + x*(bw+bm) + "," + y*(bh+bm) + ")";
}

function makebox(g, id) {
    var n = parseInt(id.substring(0,6), 16);
    g.append("rect")
        .attr("width", bw).attr("height", bh)
        .attr("rx", br).attr("ry", br)
        .attr("fill", d3.hsl(n/0xFFFFFF*359.0, .7, .7))
        .attr("stroke", "black").attr("stroke-width", bs);
    g.append("text")
        .attr("x", bw/2).attr("text-anchor", "middle")
        .attr("y", bh/2).attr("dominant-baseline", "central")
        .attr("font-family", "monospace")
        .attr("font-size", bfs)
        .html(id.substring(0,6).toUpperCase());
}

function update(err, json) {

    var items = topg.selectAll(".node").data(json.data, function(d) { return d.id; })

    items
        .transition().delay(1*td).duration(td)
        .attr("transform", function(d, i) { return bt(0, i); })
        .style("opacity", 1.0);

    items.enter()
        .append("g").classed("node", true)
        .each(function (d, i) { makebox(d3.select(this), d.id); })
        .attr("transform", function(d, i) { return bt(0, i); })
        .style("opacity", 1e-6)
        .transition().delay(2*td).duration(td)
        .style("opacity", 1.0);

    items.exit()
        .transition().delay(0*td).duration(td)
        .style("opacity", 1e-6)
        .remove();

    var keys = items.selectAll(".key").data(
        function(d, i) { return d.keys; },
        function(d) { return d; }
    );

    keys
        .transition().delay(1*td).duration(td)
        .attr("transform", function(d, i) { return bt(i+1, 0); })
        .style("opacity", 1.0);

    keys.enter()
        .append("g").classed("key", true)
        .each(function (d, i) { makebox(d3.select(this), d); })
        .attr("transform", function(d, i) { return bt(i+1, 0); })
        .style("opacity", 1e-6)
        .transition().delay(2*td).duration(td)
        .style("opacity", 1.0);

    keys.exit()
        .transition().delay(0*td).duration(td)
        .style("opacity", 1e-6)
        .remove();

    d3.json("/api/v1/nodes/ajax?t=" + tabId, update);

}

d3.json("/api/v1/nodes?t=" + tabId, update);
