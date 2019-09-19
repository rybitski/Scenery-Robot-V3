import React from 'react';
import { Tabs, Tab, Jumbotron, InputGroup, FormControl, ButtonToolbar, Button, Dropdown, Container, Col, Row, ButtonGroup } from 'react-bootstrap';


const Manual = () => {
    return(

        <div>
            <Tabs defaultActiveKey="profile" id="uncontrolled-tab-example">
            <Tab eventKey="profile" title="Presets">
           <h2> Change Presets</h2>
           <p> Only do this if you are absolutely sure, please consult the wiki for more info </p>
           <Button variant="danger">Do Stuff</Button>
           </Tab>
           <Tab eventKey="contact" title="Advanced" >
            </Tab>

           </Tabs>
        </div>
    )
}

export default Manual;