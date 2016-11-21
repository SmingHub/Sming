var express    = require('express');
var app        = express();
var bodyParser = require('body-parser');
var cors       = require('cors')
var morgan     = require('morgan');
var http       = require('http');

//Configure Express
app.use(bodyParser.urlencoded({ extended: true }));
app.use(bodyParser.json());
app.use(morgan('dev'));
app.use(cors());
var router = express.Router();

// Root URI.
router.get('/', function(req, res) {
        res.json( {message: 'REST PROXY' });
});


router.route('/test')
    .get( function(req, res) {
	console.log("GET Request sucessuful.");
	console.log("Body: " + JSON.stringify(req.body) );
	res.json( { "status":"OK GET" } );
    })
    .post( function(req, res) {
	console.log("POST Request sucessuful.");
	console.log("Body: " + JSON.stringify(req.body) );
	res.json( { "status":"OK POST" } );
    })
    .put( function(req, res) {
	console.log("PUT Request sucessuful.");
	console.log("Body: " + JSON.stringify(req.body) );
	res.json( { "status":"OK PUT" } );
    })
    .delete( function(req, res) {
	console.log("DELETE Request sucessuful.");
	console.log("Body: " + JSON.stringify(req.body) );
	res.json( { "status":"OK DELETE" } );
    });

// Our base url is /api
app.use('/api', router);
app.listen(3003);

var datenow = new Date();
console.log("========================================");
console.log("REST TEST Server started at " + datenow );
console.log("Api endpoint available at http://localhost:3003/api");

