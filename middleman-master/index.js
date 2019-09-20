const express = require('express');
const path = require('path');

const app = express();
var bodyParser = require('body-parser')
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: false }));
app.use(express.static('public'))

var xdata;
var homedata;
var cuedata;
var encodedata;
// Serve the static files from the React app
app.use(express.static(path.join(__dirname, 'client/build')));

// DEPRECATED: An api endpoint that returns a short list of items
app.get('/api/getList', (req, res) => {
    var list = ["item1", "item2", "item3"];
    res.json(list);
    console.log('Sent list of items');
});

//"Home" server page that tells the Yun what's cracking
app.post('/api/home', function (req, res) {
    homedata = req.body.data;
    res.send(req.body.data)
    console.log("Home vals = " + homedata);
    console.log("joe");
    var fs = require('browserify-fs');
   

})
//Cue page where the list is loaded
app.post('/api/cue', function (req, res) {
    cuedata = req.body.data;
    res.send(req.body.data)
    console.log("Data = " + cuedata);
})

//When the client requests to send data, accept and save it
app.post('/api/xsend', function (req, res) {
    xdata = req.body.data;
    res.send(req.body.data)
    console.log("Data = " + xdata);

})

app.post('/api/encoder', function (req, res) {
    encodedata = req.body.data;
    res.send(req.body.data)
    console.log("Encoder = " + encodedata);
})
//Put the new data on the actual server page
app.get('/api/xsend', function (req, res) {
    res.json(xdata);
})
app.get('/api/home', function (req, res) {
    res.json(homedata);
})
app.get('/api/cue', function (req, res) {
    res.json(cuedata);
})
app.get('/api/encoder', function (req, res) {
    res.json(encodedata);
})






app.get('*', (req, res) => {
    res.sendFile(path.join(__dirname + '/client/build/index.html'));
});

const port = process.env.PORT || 5000;
app.listen(port);

console.log('App is listening on port ' + port);