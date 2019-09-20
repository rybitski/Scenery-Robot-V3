import React, { Component, useState } from 'react';
import { Link } from 'react-router-dom';
import Nav from 'react-bootstrap/Nav'
import NavDropdown from 'react-bootstrap/NavDropdown'
import Form from 'react-bootstrap/Form'
import Button from 'react-bootstrap/Button'
import FormControl from 'react-bootstrap/FormControl'
import style from 'bootstrap/dist/css/bootstrap.css';
import { Container, Row, Col } from 'react-bootstrap'
import Navbar from 'react-bootstrap/Navbar'
import { Tabs, Tab, Modal } from 'react-bootstrap';

import List from './List'
import Cues from './Cue'


class Home extends Component {
  constructor(props) {
    super(props);
    this.manSelect = this.manSelect.bind(this); //add load cues
    this.cueSelect = this.cueSelect.bind(this);
    this.handleCloseModal = this.handleCloseModal.bind(this);
    this.handleShowModal = this.handleShowModal.bind(this);
    this.handleChangeIP = this.handleChangeIP.bind(this);

    this.state = {
      modal_ip: false,
      ip: "192.168.0.000",
      mode: 0,
    }

  }
  sendData = () => {
    var out = this.state.mode + "," + 0 + "," + 1;
    fetch('/api/home', { //The remaining url of the localhost:5000 server.
      method: 'post',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({
        data: out,
      }),
    })
  };
  componentDidMount() {
    var data = require('./files/prefs.json');
    //console.log(data.ip);
    this.setState({
      ip: data.ip
    });
    this.sendData();
  }
  manSelect() {
    this.setState({
      mode: 2
    });
    this.sendData()
  }
  cueSelect() {
    this.setState({
      mode: 1
    });
    this.sendData()
  }
  handleLoadCues(){
    var data = require('./cuelisttest.json');
    this.setState({
        cues: data
    });
    this.sendData();
  } 
  handleShowModal(){
    this.setState({
      modal_ip: true
    });
  }
  handleCloseModal(){
    this.setState({
      modal_ip: false
    });
    var fs = require('browserify-fs');
    var datatest = {
      name: "cliff",
      age: "34",
      name: "ted",
      age: "42",
      name: "bob",
      age: "12"
    }

  var jsonData = JSON.stringify(datatest);
  console.log("joe");
  fs.mkdir('/test', function() {
    fs.writeFile('/test/hello-world.txt', 'Hello world!\n', function() {
        fs.readFile('/test/hello-world.txt', 'utf-8', function(err, data) {
            console.log(data);
        });
    });
  });

  }
  handleChangeIP(event){
    this.setState({
      ip: event.target.value
    });
  }


  render() {
    return (
      <div className="App">
          <Navbar bg="light" expand="lg">
            <Navbar.Brand href="#home">Robot UI v2</Navbar.Brand>
            <Navbar.Toggle aria-controls="basic-navbar-nav" />
            <Navbar.Collapse id="basic-navbar-nav">
              <Nav className="mr-auto">
                <Nav.Link href="#home">Home</Nav.Link>
                <Nav.Link href="https://github.com/rybitski/Scenery-Robot-V3/wiki">Wiki</Nav.Link>
                <NavDropdown title="Connections" id="basic-nav-dropdown">
                  <NavDropdown.Item onClick={this.handleShowModal}>Start a New Connection</NavDropdown.Item>
                  <NavDropdown.Item href="#action/3.2">Test Connection</NavDropdown.Item>
                  <NavDropdown.Item href="#action/3.3">Get Info</NavDropdown.Item>
                </NavDropdown>
                <NavDropdown title="Cue List" id="basic-nav-dropdown">
                  <NavDropdown.Item href="#action/3.1">Recent Cues</NavDropdown.Item>
                  <NavDropdown.Item onClick={this.handleLoadCues}>Load New Set</NavDropdown.Item>
                </NavDropdown>
              </Nav>
              <Form inline>
                <FormControl type="text" placeholder="Search" className="mr-sm-2" />
                <Button variant="outline-success">Search</Button>
              </Form>
            </Navbar.Collapse>
          </Navbar>
        {/* Link to List.js */}
        <Container>
          <Modal show={this.state.modal_ip} onHide={this.handleCloseModal}>
            <form onSubmit={this.handleCloseModal}> 
              <Modal.Header closeButton>
                <Modal.Title>Modal heading</Modal.Title>
              </Modal.Header>
              <Modal.Body>
                <div className="form-group col-md-6">
                  IP:
                  <input type="text" value={this.state.ip} onChange={this.handleChangeIP} className="form-control" />
                </div>    
              </Modal.Body>          
              <Modal.Footer>
                <Button variant="secondary" onClick={this.handleCloseModal}>
                  Close
                </Button>
                <Button variant="primary" onClick={this.handleCloseModal}>
                  Save Changes
                </Button>
              </Modal.Footer>
            </form>
          </Modal> 
          <Col>
            <Row> 
              <Col>
                <Link to={'./cue'}>
                  <button variant="raised" onClick={this.cueSelect}>
                    Start Cue System
                  </button>
                </Link>
              </Col>
              <Col>
                <Tabs defaultActiveKey="cue" id="uncontrolled-tab-example">
                  <Tab eventKey="man" title="Manual Control">
                    <h2> Manual Control</h2>
                    <p> Only do this if you are absolutely sure, please consult the wiki for more info </p>
                    <Button variant="danger">Activate the Giant Red Button</Button>
                    <List/>
                  </Tab>
                  <Tab eventKey="cue" title="Cue Stuff" >
                    <Cues/>
                  </Tab>
                </Tabs>
              </Col>
            </Row>
            <Row>
                       
            </Row>
          </Col>

        </Container>




      </div>
    )
  }
}
export default Home;