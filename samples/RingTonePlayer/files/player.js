var websocket;
var connected = false;
var connectionTimeout;
var graphCpuUsage;
var graphFillTime;
var graphMem;

function initPlayer() {
	drawVoice("Sine");
	initGraphs();
	wsInit();
}


function wsInit() {
	if(location.host == '') {
		$("#spinner").hide();
		return;
	}

	$("#spinner").show();

	var wsUri = "ws://" + location.host + "/ws";
	websocket = new WebSocket(wsUri);

	websocket.onopen = function(evt) {
		onOpen(evt);
	};

	websocket.onclose = function(evt) {
		onClose(evt);
	};

	websocket.onmessage = function(evt) {
		onMessage(evt);
	};

	websocket.onerror = function(evt) {
		onError(evt);
	};
}

function resetConnectionTimeout() {
	clearTimeout(connectionTimeout);
	connectionTimeout = setTimeout(function() {
		console.log('Connection timeout');
		// Don't bother waiting for a close to complete
		try {
			websocket.onerror = null;
			websocket.onclose = null;
			websocket.close();
			websocket = null;
		}
		catch(err) {
		}
		wsInit();
	}, 5000);
}

function onOpen(evt) {
	connected = true;
	$("#spinner").hide();
	resetConnectionTimeout();
	listTunes();
}

function onClose(evt) {
	showAlert("Disconnected", 'warning');
	websocket = null;
	clearTimeout(connectionTimeout);
	connectionTimeout = null;
	setTimeout(wsInit, 3000);
}

function onMessage(evt) {
	resetConnectionTimeout();
	var json = JSON.parse(evt.data);
	var cpu = json.Cpu;
	$.each(json, function(code, msg) {
		switch (code) {
		case "Time":
			var secs = Math.round(msg / 1000);
			var mins = Math.floor(secs / 60);
			secs = secs % 60;
			if(secs < 10) secs = '0' + secs;
			if(mins < 10) mins = '0' + mins;
			$("#time").text(mins + ':' + secs);
			break;
		case "Info":
			showAlert(msg.text, 'info');
			break;
		case "Warning":
			showAlert(msg.text, 'warning');
			break;
		case "Error":
			showAlert(msg.text, 'danger');
			break;
		case "TuneChanged":
			$("#listButton").text(msg.text);
			$("#tune-num").val(msg.index);
			break;
		case "TuneCompleted":
			break;
		case "Started":
			$("#start").attr('disabled', true);
			$("#pause").attr('disabled', false);
			$("#stop").attr('disabled', false);
			break;
		case "Paused":
			$("#start").attr('disabled', false);
			$("#pause").attr('disabled', true);
			$("#stop").attr('disabled', false);
			break;
		case "Stopped":
			$("#start").attr('disabled', false);
			$("#pause").attr('disabled', false);
			$("#stop").attr('disabled', true);
			break;
		case "VoiceChanged":
			drawVoice(msg.text);
			break;
		case "ModeChanged":
			var mode = msg.text;
			if(mode == 'Random') {
				mode = '&#128256';
			} else if(mode == 'Sequential') {
				mode = '&#8674;';
			}
			$("#mode").html(mode);
			break;
		case "SpeedChanged":
			var speed = (msg.text / 100.0).toFixed(2);
			$("#speed").val(speed);
			$("#speed-range").val(speed);
			break;
		case "Cpu":
			var usage = msg.usage / 100;
			setProgress($("#cpu-usage"), usage, 100, '%');
			var graph = graphCpuUsage;
			graph.traces[1].addValue(usage);
			graph.draw();
			break;
		case "FillTime":
			setProgress($("#fill-time"), 100 * msg.total / cpu.cycles, 5, '%');
			var scale = function(n) {
				return 1000 * n / cpu.cycles;
			};
			var graph = graphFillTime;
			graph.traces[0].addValue(scale(msg.min * msg.count));
			graph.traces[1].addValue(scale(msg.total));
			graph.traces[2].addValue(scale(msg.max * msg.count));
			graph.draw();
			break;
		case "Mem":
			setProgress($("#mem"), msg.free / 1024, 50, 'KB');
			var scale = function(n) {
				return n / 1024;
			};
			var graph = graphMem; 
			graph.traces[0].addValue(scale(msg.used));
			graph.traces[1].addValue(scale(msg.free));
			graph.traces[2].addValue(scale(msg.peak));
			graph.draw();
			break;
		}
	});
}

function setProgress($progressBar, value, scale, unit = '') {
	$progressBar.css('width', (100 * value / scale).toFixed(2) + '%');
	$progressBar.text(value.toFixed(2) + ' ' + unit);
}

function onError(evt) {
	showAlert('Websocket error', 'danger');
}

function showAlert(message, alertType) {
	var obj = $('<div id="alertdiv" class="col alert alert-' +  alertType + ' fade show"><span>' + message + '</span></div>');
	$('#alert-placeholder').append(obj);

	setTimeout(function(obj) {
		obj.alert('close');
	}, 5000, obj);
}

function sendCommand(cmd, arg) {
	if(!connected) {
		showAlert("Not Connected", 'danger');
		return;
	}

	var json = {};
	json.cmd = cmd;
	json.arg = arg;
	var s = JSON.stringify(json);
	try {
 		websocket.send(s);
	}
	catch(err) {
		showAlert(err.message, 'danger');
		websocket = null;
	}
}

function goto() {
	sendCommand('goto', $('#tune-num').val());
}

function setSpeed() {
	sendCommand('speed', $('#speed').val());
}

function listTunes() {
	$.getJSON('ajax/list', function(data) {
		var list = $("#list");
		list.empty();
		$.each(data.list, function(key, val) {
			list.append('<a class="dropdown-item tune" href="#" index="' + key + '">' + key + ': ' + val + '</a>');
		});
	});
}

function polyLine(ctx, pts) {
	ctx.moveTo(pts[0], pts[1]);
	for(i=2; i<pts.length; i+=2) {
		ctx.lineTo(pts[i], pts[i+1]);
	}
}

function drawVoice(voice)
{
	var canvas = $("#canvas-voice")[0];
	var ctx = canvas.getContext("2d");
	ctx.setTransform(1, 0, 0, 1, 0, 0);
	ctx.clearRect(0, 0, canvas.width, canvas.height);
	ctx.setTransform(canvas.width - 3, 0, 0, canvas.height - 3, 2, 2);

	// Grid
	ctx.beginPath();
	ctx.strokeStyle = "lightgray";
	ctx.lineWidth = 0.02;
	polyLine(ctx, [0,0.5, 1,0.5]);
	polyLine(ctx, [0.5,0, 0.5,1]);
	ctx.strokeRect(0,0, 1,1);
	ctx.stroke();

	// Waveform
	ctx.strokeStyle = "white";
	ctx.lineWidth = 0.1;
	ctx.beginPath();
	switch(voice) {
	case "Sine":
		ctx.moveTo(0,0.5);
		ctx.quadraticCurveTo(0.25, -0.5, 0.5,0.5);
		ctx.quadraticCurveTo(0.75, 1.5, 1,0.5);
		break;
	case "Triangular":
		polyLine(ctx, [0,1, 0.25,0, 0.5,1, 0.75,0, 1,1]);
		break;
	case "Sawtooth":
		polyLine(ctx, [0,1, 0.5,0, 0.5,1, 1,0, 1,1]);
		break;
	case "Square":
		polyLine(ctx, [0,1, 0,0, 0.5,0, 0.5,1, 1,1, 1,0]);
		break;
	}
	ctx.stroke();
}

function initGraphs()
{
	const graphValueCount = 120;

	var traces = [
		{name: "min", strokeStyle: "green"},
		{name: "average", strokeStyle: "black"},
		{name: "max", strokeStyle: "red"}
	];

	graphCpuUsage = new Graph($("#graph-cpu-usage")[0]);
	graphCpuUsage.name = "CPU Usage";
	graphCpuUsage.valueCount = graphValueCount;
	graphCpuUsage.addTraces(traces);
	
	graphFillTime = new Graph($("#graph-fill-time")[0]);
	graphFillTime.name = "Fill Times";
	graphFillTime.valueCount = graphValueCount;
	graphFillTime.addTraces(traces);

	graphMem = new Graph($("#graph-mem")[0]);
	graphMem.name = "Free Memory";
	graphMem.valueCount = graphValueCount;
	graphMem.addTraces(traces);

	sizeGraphs = function() {
		for(g of [graphCpuUsage, graphFillTime, graphMem]) {
			g.canvas.width = g.canvas.clientWidth;
			g.draw();
		}
	};

	sizeGraphs();
	$(window).on('resize', sizeGraphs);

	if(location.host == '') {
		for(g of [graphCpuUsage, graphFillTime, graphMem]) {
			testFillGraph(g);
			g.draw();
		}
		setProgress($("#cpu-usage"), Math.random() * 100, 100, '%');
		setProgress($("#fill-time"), Math.random() * 5, 5, '%');
		setProgress($("#mem"), Math.random() * 50, 50, 'KB');
	}
}

function testFillGraph(graph)
{
	var maxValue = Math.random() * 100;
	var m = maxValue / 3; 

	var trace = graph.traces[0];
	for(i = 0; i < graph.valueCount; ++i) {
		trace.addValue(Math.random() * m);
	}

	trace = graph.traces[1];
	for(i = 0; i < graph.valueCount; ++i) {
		trace.addValue(m + Math.random() * m);
	}

	trace = graph.traces[2];
	for(i = 0; i < graph.valueCount; ++i) {
		trace.addValue(2 * m + Math.random() * m);
	}
}
