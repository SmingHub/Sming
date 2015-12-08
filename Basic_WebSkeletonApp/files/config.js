function get_config() {
	$.getJSON('/config.json',
			function(data) {
				$.each(data, function(key, value){
            		document.getElementById(key).value = value;
            	if (data.StaEnable == 1) {
            		document.getElementById('StaEnable').checked = true;
            	}
            	else
            		document.getElementById('StaEnable').checked = false;
        		});
            });
}


function post_netcfg(event) {
	event.preventDefault();
	var formData = {
			'StaSSID'					:	document.getElementById('StaSSID').value,
			'StaPassword'				:	document.getElementById('StaPassword').value,
			'StaEnable'					:	(document.getElementById('StaEnable').checked ? 1 : 0)
			};
	$.ajax({
        type        : 'POST',
        url         : '/config',
        contentType	: 'application/json; charset=utf-8',
        data        : JSON.stringify(formData),
        dataType	: 'json'
    })
}


$( document ).ready(function() {
	get_config();
	
	document.getElementById('form_netcfg').addEventListener('submit', post_netcfg);
	document.getElementById('netcfg_cancel').addEventListener('click', get_config);
});