const express = require('express');
const path = require('path');

const app = express();
var bodyParser = require('body-parser')
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: false }));
app.use(express.static('public'))

var xdata; 

// Serve the static files from the React app
app.use(express.static(path.join(__dirname, 'client/build')));

// DEPRECATED: An api endpoint that returns a short list of items
app.get('/api/getList', (req,res) => {
    var list = ["item1", "item2", "item3"];
    res.json(list);
    console.log('Sent list of items');
});

//Put the new data on the actual server page
app.get('/api/xsend', function (req, res) {
    res.json(xdata);


})

//When the client requests to send data, accept and save it
app.post('/api/xsend', function (req, res) {
    xdata = req.body.data;

    res.send(req.body.data)
    console.log("Data = "+xdata);

})



app.get('*', (req,res) =>{
    res.sendFile(path.join(__dirname+'/client/build/index.html'));
});

const port = process.env.PORT || 5000;
app.listen(port);

console.log('App is listening on port ' + port);