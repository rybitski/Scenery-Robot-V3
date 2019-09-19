import React, { Component } from 'react';
import Gamepad from 'react-gamepad'
import 'rc-slider/assets/index.css';
import 'rc-tooltip/assets/bootstrap.css';
import Tooltip from 'rc-tooltip';
import Slider from 'rc-slider';
const wrapperStyle = { width: 400, margin: 25 };

const createSliderWithTooltip = Slider.createSliderWithTooltip;
const Handle = Slider.Handle;
const handle = (props) => {
    const { value, dragging, index, ...restProps } = props;
    return (
        <Tooltip
            prefixCls="rc-slider-tooltip"
            overlay={value}
            visible={dragging}
            placement="top"
            key={index}
        >
            <Handle value={value} {...restProps} />
        </Tooltip>
    );
};

class List extends Component {
    // Initialize the state
    constructor(props) {
        super(props);
        this.onRT = this.onRT.bind(this);
        this.onA = this.onA.bind(this);
        this.axisChangeHandler = this.axisChangeHandler.bind(this);
        this.onButtonDown = this.onButtonDown.bind(this);
        this.onLB = this.onLB.bind(this);
        this.onRB = this.onRB.bind(this);
        this.onButtonUp = this.onButtonUp.bind(this);
        this.handleChangeDeadzone = this.handleChangeDeadzone.bind(this);
        this.onSensitivityChange = this.onSensitivityChange.bind(this);
        this.onSpeedChange = this.onSpeedChange.bind(this);

        this.state = {
            deadzone: 0.02,
            speed: 127,
            sensitivity:0.5, 
            out_data: "",
            a_button: 0,
            rsx: 0,
            rsy: 0,
            lsx: 0,
            lsy: 0,
            rt: 0,
            dpad: 0,
            lb: 0,
            rb: 0,
            seconds: 0,
            list: []
        }

    }
    //Timer code to make sure server data is new
    tick() {
        this.setState(prevState => ({
            seconds: prevState.seconds + 0.1
        }));
        this.sendData();

    }




    connectHandler(gamepadIndex) {
        console.log(`Gamepad ${gamepadIndex} connected !`)
    }

    disconnectHandler(gamepadIndex) {
        console.log(`Gamepad ${gamepadIndex} disconnected !`)
    }
    onSpeedChange(value) {
        this.setState({ speed: value * 127 / 100 }); //normalize the speed from a 1-100 percentage to 1-127 speed 

    }
    onSensitivityChange(value){
        this.setState({ sensitivity: value / 100 });

    }

    handleChangeDeadzone(event) {
        this.setState({ deadzone: event.target.value });
        console.log(this.state.deadzone)
    }

    handleSubmitDeadzone(event) {
        console.log('Your favorite flavor is: ' + this.state.deadzone);
    }

    //When button is released, set the value to 0

    onButtonUp(buttonName) {
        if (buttonName === "A")
            this.setState({
                a_button: 0
            });
        if (buttonName === "LB")
            this.setState({
                lb: 0
            });
        if (buttonName === "RB")
            this.setState({
                rb: 0
            });
        if (buttonName === "DPadDown")
            this.setState({
                dpad: 0
            });
        if (buttonName === "DPadUp")
            this.setState({
                dpad: 0
            });
        if (buttonName ==="RT")
            this.setState({
                rt: 0
            });    }

    //Stick handler: for x-values (turning) cast to an exact value between -127 and 127. This is to eliminate jerkiness in turning
    //For the y-values (forward/back), cast to 127 then divide by 10. This reduces precision, but also lowers the number of packets to be sent.
    axisChangeHandler(axisName, value, previousValue) {
        console.log(axisName, value)
        if (axisName === "RightStickX")
            this.setState({
                rsx: value * 127
            });
        if (axisName === "RightStickY")
            this.setState({
                rsy: value * 127 / 10
            });
        if (axisName === "LeftStickX"){
            /*var xtest = value * 127;
             if (Math.abs(xtest) > this.state.speed) { //if speed exceeds the set max, set the speed to the given maximum
                 if (value < -10)
                     this.setState({
                         lsx: this.state.speed * -1
                     });
                 else if (value > 10)
                     this.setState({
                         lsx: this.state.speed
                     });
             }
             else
                 this.setState({
                     //lsx: value * 127

                        lsx: (this.state.sensitivity*(Math.pow(value,3)) + (1-this.state.sensitivity)*value) * 127 //adding the variable sensitivity to x values
                 });*/
                 this.setState({
                    lsx: this.state.speed * value
                });
         }
        if (axisName === "LeftStickY") {
           var test = value * 127;
           /* if (Math.abs(test) > this.state.speed) { //if speed exceeds the set max, set the speed to the given maximum
                if (value < 0)
                    this.setState({
                        lsy: this.state.speed * -1
                    });
                else 
                    this.setState({
                        lsy: this.state.speed
                    });
            }
            else
                this.setState({
                    lsy: value * 127
                });*/
                this.setState({
                    lsy: this.state.speed * value
                });
        }

    }
    onRT() {
        console.log("RT")
        this.setState({
            rt: 1
        });

    }

    //When dpad is pressed, set the value to specific int

    onButtonDown(buttonName) {
        console.log(buttonName)
        if (buttonName === "DPadUp")
            this.setState({
                dpad: 1
            });
        if (buttonName === "DPadRight")
            this.setState({
                dpad: 2
            });
        if (buttonName === "DPadDown")
            this.setState({
                dpad: 3
            });
        if (buttonName === "DPadLeft")
            this.setState({
                dpad: 4
            });
    }


    onA() {
        console.log("A")
        this.setState({
            a_button: 1
        });
    }


    onLB() {
        console.log("LB")
        this.setState({
            lb: 1
        });
    }
    onRB() {
        console.log("RB")
        this.setState({
            rb: 1
        });

    }
    // Start timer on page load
    componentDidMount() {
        this.updateHome();
        this.interval = setInterval(() => this.tick(), 100);
    }

    componentWillUnmount() {
        clearInterval(this.interval);
    }

    updateHome = () => {
        var out = 2+","+0;
        fetch('/api/home', { //The remaining url of the localhost:5000 server.
            method: 'post',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                data: out,
            }),
        })
    };

    // Send data to the server. We round so that we dont send the 15 trailing decimals.
    sendData = () => {
        var out = Math.round(this.state.lsx) + "," + Math.round(this.state.lsy)  + "," + Math.round(this.state.rsy) * 10 + "," + this.state.rt + "," + this.state.dpad + "," + this.state.a_button + "," + this.state.rb + "," + this.state.lb + "," + Math.round(this.state.seconds * 10);
        fetch('/api/xsend', { //The remaining url of the localhost:5000 server.
            method: 'post',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                data: out,
            }),
        })
    };

    //What will render on the client page
    render() {
        const { list } = this.state;

        return (
            <div className="App">
                <Gamepad
                    deadZone={this.state.deadzone}
                    onConnect={this.connectHandler}
                    onDisconnect={this.disconnectHandler}
                    onButtonDown={this.onButtonDown}
                    onAxisChange={this.axisChangeHandler}
                    onLB={this.onLB}
                    onRB={this.onRB}
                    onA={this.onA}
                    onRT={this.onRT}
                    onButtonUp={this.onButtonUp}

                >
                    <p></p>
                </Gamepad>
                <div>
                    Seconds: {this.state.seconds}

                </div>
                <div style={wrapperStyle}>
                    <p>Speed</p>
                    <Slider min={0} max={100} defaultValue={100} onChange={this.onSpeedChange} handle={handle} />
                </div>
                <form onSubmit={this.handleSubmitDeadzone}>
                    <label>
                        Pick your deadzone:
                        <select value={this.state.deadzone} onChange={this.handleChangeDeadzone}>
                            <option value="0.02">2</option>
                            <option value="0.04">4</option>
                            <option value="0.06">6</option>
                            <option value="0.08">8</option>
                        </select>
                    </label>
                </form>
                <div style={wrapperStyle}>
                    <p>Sensitivity</p>
                    <Slider min={0} max={100} defaultValue={50} onChange={this.onSensitivityChange} handle={handle} />
                </div>
                <h1>Live Server Data</h1>
                {/* Check to see if any items are found*/}
                {list.length ? (
                    <div>
                        {/* Render the list of items */}
                        {list.map((item) => {
                            return (
                                <div>
                                    {item}
                                </div>
                            );
                        })}
                    </div>
                ) : (
                        <div>
                            <h2>Server Stuff</h2>

                            {Math.round(this.state.lsx)},{Math.round(this.state.lsy)},{Math.round(this.state.rsy) * 10},{this.state.rt},{this.state.dpad},{this.state.a_button},{this.state.rb},{this.state.lb}, {this.state.deadzone}, {this.state.speed}, {this.state.sensitivity}
                        </div>
                    )
                }
            </div>
        );
    }

}

export default List;