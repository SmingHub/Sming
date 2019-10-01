function Trace(graph) {
	this.graph = graph;
	this.values = [];
	this.strokeStyle = "black";
	this.maxValue = 0;

	this.addValue = function(value) {
		if(this.values.length == 0) {
			this.maxValue = value;
		} else {
			if (this.values.length >= this.graph.valueCount) {
				this.values.shift();
			}
			this.maxValue = Math.max(this.maxValue, value);
		}
		this.values.push(value);
	};
}

/*
 * A very simple line graph class
 */
function Graph(canvas) {
	this.canvas = canvas;
	this.traces = [];
	this.valueCount = 10;
	this.maxValue = 10;
	this.gridIntervalX = 10;
	this.gridIntervalY = 10;
	const xo = 20;
	const yo = 12;

	this.addTrace = function(name, strokeStyle) {
		var trace = new Trace(this);
		trace.strokeStyle = strokeStyle;
		this.traces.push(trace);
		return trace;
	};

	this.addTraces = function(list) {
		for(cfg of list) {
			this.addTrace(cfg.name, cfg.strokeStyle);
		}
	};

	this.draw = function() {
		var w = this.canvas.width;
		var h = this.canvas.height;
		var ctx = this.canvas.getContext("2d");
		ctx.setTransform(1, 0, 0, 1, 0, 0);
		ctx.clearRect(0, 0, w, h);

		h -= yo * 2;
		w -= xo;
		ctx.translate(xo, yo)

		for (trace of this.traces) {
			this.maxValue = Math.max(this.maxValue, trace.maxValue);
		}

		this.yCalc = function(value) {
			return h * (1 - (value / this.maxValue));
		};

		this.xCalc = function(index) {
			return w * index / (this.valueCount - 1);
		};

		// draw grid
		ctx.strokeStyle = "lightgray";
		ctx.fillStyle = "gray";
		ctx.textBaseline = "middle";
		ctx.textAlign = "end";
		ctx.lineWidth = 0.5;
		ctx.beginPath();
		var intervals = Math.ceil(this.maxValue / this.gridIntervalY);
		this.maxValue = intervals * this.gridIntervalY; 
		for (i = 0; i <= intervals; ++i) {
			var n = i * this.gridIntervalY;
			var y = this.yCalc(n);
			ctx.moveTo(0, y);
			ctx.lineTo(w, y);
			ctx.fillText(n, -5, y);
		}
		var intervals = Math.ceil(this.valueCount / this.gridIntervalX);
		for (i = 0; i <= intervals; ++i) {
			var x = this.xCalc(i * this.gridIntervalX);
			ctx.moveTo(x, 0);
			ctx.lineTo(x, h);
		}
		ctx.stroke();

		ctx.lineWidth = 0.6;
		for (trace of this.traces) {
			ctx.strokeStyle = trace.strokeStyle;
			ctx.beginPath();
			for (i = 0; i < this.valueCount; ++i) {
				var x = this.xCalc(i);
				var y = this.yCalc(trace.values[i]);
				if (i == 0) {
					ctx.moveTo(x, y);
				} else {
					ctx.lineTo(x, y);
				}
			}
			ctx.stroke();
		}
	};
}
