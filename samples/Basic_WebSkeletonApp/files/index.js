'use strict';

function updateState() {
	fetch('/state.json')
  	.then(function(response) {
      if (response.status == 200) return response.json();
   })
  .then(function(json) {
    document.getElementById('counter').textContent = json.counter;
  });
}

//Here we put some initial code which starts after DOM loaded
function onDocumentRedy() {
	//Init
	updateState();
	setInterval(updateState, 5000);

}

document.addEventListener('DOMContentLoaded', onDocumentRedy);