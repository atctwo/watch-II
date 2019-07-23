# watch II state system_states
watch II uses a state system to decide what "app" is currently being executed.  State 0 is in initalisation state, state 1 is the watch face, and state 2 is the state menu.  You don't see anything done by state 0, because it executes once, then changes to state 1.

## Adding system states
One way that you can add your own program to the system is to add your own state.  The other option is to write a Lua script.  If you decide to write a state, you get more control over the hardware, and probably more performance, but you have to recompile the system (which you don't have to do if you write a Lua script), and you have to write the state in C++ (specifically Arduino).

To write a state, open `watch2/states/user_states.cpp`.  You will notice that the file contains a function called `registerUserStates()`.  This function is called during system initialisation, so any code written inside this function is executed.  To add a state, you have to use a system function called `registerState()`.  Its parameters are described below:

```
int registerState(String stateName,
                  const std::function<void()> &stateFunc );

    stateName       The name of the state as shown on the state menu
    stateFunc       The function to be executed when this state is loaded

```
Whenever a state is chosen from the state menu, the state is switched, and the function attributed to that state is executed.  The function is executed in a loop until the state is changed.  So that you know when the state is being executed for the first time, you can use a global variable called `state_init`.  Normally this variable is set to 0 when the state function is being called for the first time, and is set to 1 for each call thereafter.  Note that what you write to the screen on the first call won't be displayed, but every subsequent call will.

You can write a function, and pass it as the argument stateFunc, or you can use a lambda.  Rather than writing a function, and giving it a name, and then passing it to `registerState()`, you can write a function as part of the call to `registerState()`.  This is shown below:

```

// define a function and pass it to registerState
void coolStateFunc()
{
    oled.setCursor(2, 2);
    oled.setTextColor(WHITE);
    oled.println("Hello from this function");
}
registerState("A Cool State, using a defined function", coolStateFunc);

// use a lambda
registerState("A Cool State, using a lambda", [](){
    oled.setCursor(2, 2);
    oled.setTextColor(WHITE);
    oled.println("Hello from this lambda");
});

```

You can read up on lambdas on many C++ tutorial websites.

## Changing states
To switch state, use the function `switchState()`.  It has one parameter, which is the state you want to switch to.  If you are returning to the main menu, switch to state number 2.

The state is specified by a global variable called `state`.  There is noting stopping you manually setting it, but that might be a bad idea.
