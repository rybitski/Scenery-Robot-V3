import React, { Component } from 'react';
import { Link } from 'react-router-dom';
import Nav from 'react-bootstrap/Nav'
import NavDropdown from 'react-bootstrap/NavDropdown'
import Form from 'react-bootstrap/Form'
import Button from 'react-bootstrap/Button'
import FormControl from 'react-bootstrap/FormControl'
import style from 'bootstrap/dist/css/bootstrap.css';
import { Container, Row, Col } from 'react-bootstrap'
import Navbar from 'react-bootstrap/Navbar'
import { Tabs, Tab } from 'react-bootstrap';

import List from './List'
import Cues from './Cue'


class Home extends Component {
  constructor(props) {
    super(props);
    this.manSelect = this.manSelect.bind(this);
    this.cueSelect = this.cueSelect.bind(this);

    this.state = {
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


  render() {
    return (
      <div className="App">
          <Navbar bg="light" expand="lg">
            <Navbar.Brand href="#home">Robot UI v2</Navbar.Brand>
            <Navbar.Toggle aria-controls="basic-navbar-nav" />
            <Navbar.Collapse id="basic-navbar-nav">
              <Nav className="mr-auto">
                <Nav.Link href="#home">Home</Nav.Link>
                <Nav.Link href="/wiki">Wiki</Nav.Link>
                <NavDropdown title="Connections" id="basic-nav-dropdown">
                  <NavDropdown.Item href="#action/3.1">Start a New Connection</NavDropdown.Item>
                  <NavDropdown.Item href="#action/3.2">Test Connection</NavDropdown.Item>
                  <NavDropdown.Item href="#action/3.3">Get Info</NavDropdown.Item>
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

        </Container>




      </div>
    )
  }
}
export default Home;