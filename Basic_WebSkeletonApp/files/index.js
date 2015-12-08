$( document ).ready(function() {
	
	(function worker() {
		$.getJSON('/state', function(data) {
			document.getElementById('counter').textContent = data.counter;
			setTimeout(worker, 5000);
		});
	})();
});