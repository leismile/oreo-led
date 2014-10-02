# System Architecture 


        +------------------+        +----------------+
        |                  |  tick  |                |
        |                  < - - - -+  Waveform Gen  |---> To LEDs
        |                  |        |                |
        |                  |        +--------^-------+
        |                  |                 |        
        |   Synchronized   |                 |        
        |   Clock          |                 |        
        |                  |        +--------+-------+
        |                  |  theta |                |
        |                  +-------->  Pattern       |
        |                  |        |  Generator     |
        |                  |        |                |
        +--------^---------+        +--------^-------+
                 |                           |        
                 |                           |        
                 |                           |        
                 |                  +--------+-------+
                 |                  |                |
                 |                  |  Lighting      |
                 |                  |  Interface     |
                 |                  |  Protocol      |
                 |                  |                |
                 |                  +--------^-------+
                 |                           |        
                 |                           |        
                 |                           |        
        +--------+---------------------------+-------+
        |                                            |
        |                TWI Manager                 |
        |                                            |
        +--------------------------------------------+


# Prioritized Tasks

    [x] fadeOut pattern
    [x] fadeIn pattern
    [x] colorCycle pattern
    [x] phase parameter
    [x] color setting limits high and low

    [ ] create fake pixhawk, controlled via serial application
    [x] create parameters for pattern coefficients
    [x] repeat parameter
    [ ] apply repeate for all patterns
    [x] refactor light manager, breakout to more classes
    [x] improve twi feature encapsulation with light manager

    [x] implement command parameter parser
    [x] pattern speed parameter overflows TOP limit in pattern implementation of theta

# Issues

    [x] devices in sync, but not steady frequency
        - fixed phase signal from arduino; set to correct period
    [x] un-addressed devices are activating SIREN pattern
        - fixed logic error in refactored TWI addressing code
    [ ] slave holds sda line high and stops all communication
    [ ] slaves freeze after a sequenctial phase offset command to each unit
    [x] turn off lights completely at zero intensity
    [ ] increase pwm freq to improve low end dithering
    [ ] verify synchronization is working with a solid phase signal

# Deferred

    [ ] make proportional adjustments to phase error
    [ ] hue parameters
    [ ] implement parameter read functions
    [ ] physical lines need to set I2C address
    [x] use physical i2c addresses, remove subaddr



