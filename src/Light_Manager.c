/*
 * Light_Manager.c
 *
 */

#include <avr/io.h>
#include <avr/cpufunc.h>
#include "Light_Manager.h"
#include "math.h"

LightManager _manager_instance;

/*
 * setup lighting manager
 * params are RGB channel output registers
 */
void LightManager_init(volatile uint8_t* output_r, volatile uint8_t* output_g, volatile uint8_t* output_b) {

    _manager_instance.isCommandFresh          = 0;
    _manager_instance.systemPhase           = 0;
    _manager_instance.devicePhaseError      = 0;
    _manager_instance.devicePhaseCorrection = 0;

    // set output channel addresses
    _manager_instance.output_r = output_r;
    _manager_instance.output_g = output_g;
    _manager_instance.output_b = output_b;

    // initialize output channels to WHITE
    *(_manager_instance.output_r) = 30;
    *(_manager_instance.output_g) = 230;
    *(_manager_instance.output_b) = 80;

    // init light pattern
    LightManager_setPattern(PATTERN_NONE);

}

void LightManager_setCommandUpdated() {

    _manager_instance.isCommandFresh = 1;

}

void LightManager_parseCommand(char* twiCommandBuffer) {

    if (_manager_instance.isCommandFresh) {

        // set pattern
        LightManager_setPattern( twiCommandBuffer[0] );

    }

    _manager_instance.isCommandFresh = 0;

}

/* 
 * set the current pattern
 */
void LightManager_setPattern(LightManagerPattern pattern) {

    // set current pattern
    _manager_instance.currPattern = pattern;

}

/* 
 * set the counter
 */
void LightManager_setCounter(int16_t count) {

    // set counter
    _manager_instance.patternCounter = count;

}

/* 
 * set the phase
 */
void LightManager_setPhase(int16_t phase) {

    // set phase
    _manager_instance.patternPhase = phase;

}

/* 
 * set the speed
 */
void LightManager_setSpeed(int16_t speed) {

    // set speed
    _manager_instance.patternSpeed = speed;

}

void LightManager_recordPhaseError() {

    _manager_instance.systemPhase = _manager_instance.patternCounter;

}

void LightManager_tickSkip() {
    _manager_instance.skipNextTick++;
}

void LightManager_updatePhaseCorrection() {
    // recompute phase correction as soon as possible
    _manager_instance.devicePhaseCorrectionUpdated = 0;
}

/* 
 * update light effects time counter
 */
void LightManager_tick() {

    // adjust clock for phase error
    if (_manager_instance.skipNextTick > 0) { 
        _manager_instance.skipNextTick--;
        return;
    }

    // increment lighting pattern effect clock
    if (_manager_instance.patternCounter < PATTERN_COUNT_TOP)
        _manager_instance.patternCounter += PATTERN_COUNT_INCREMENT;
    else 
        _manager_instance.patternCounter = PATTERN_COUNT_BOTTOM;

}


// proportional correction for phase error
// limit execution to once per clock tick
void LightManager_calcPhaseCorrection() {

    // phase correction already updated in this cycle
    if (_manager_instance.devicePhaseCorrectionUpdated) return;

    // calculate phase error if a phase signal received
    if (_manager_instance.systemPhase != 0) {

        // device is behind phase
        if (_manager_instance.systemPhase >= PATTERN_COUNT_TOP/2) {
            _manager_instance.devicePhaseError = (_manager_instance.systemPhase - PATTERN_COUNT_TOP) / PATTERN_COUNT_INCREMENT;

        // device is ahead of phase
        } else {
            _manager_instance.devicePhaseError = _manager_instance.systemPhase / PATTERN_COUNT_INCREMENT;
        }

        // do not calculate phaseError again until another phase signal received
        _manager_instance.systemPhase = 0;

    }

    // only apply correction if magnitude is greater than threshold
    if (fabs(_manager_instance.devicePhaseError) >= PHASE_CORRECTION_THRESHOLD) { 

        // if phase error negative, add an extra clock tick
        // if positive, remove a clock tick
        if (_manager_instance.devicePhaseError < 0) {
            LightManager_tick();
            _manager_instance.devicePhaseError++;
        } else {
            LightManager_tickSkip();
            _manager_instance.devicePhaseError--;
        }        
        
    }

    // phase correction has been updated
    _manager_instance.devicePhaseCorrectionUpdated = 1;

}

void LightManager_calc() { 

    switch(_manager_instance.currPattern) {

        case PATTERN_SINE: 
            LightManager_patternSine(); 
            break;
        case PATTERN_STROBE: 
            LightManager_patternStrobe(); 
            break;
        case PATTERN_SIREN: 
            LightManager_patternSiren(); 
            break;
        case PATTERN_SOLID: 
            LightManager_patternSolid(); 
            break;
        case PATTERN_NONE:
        default: 
            LightManager_patternOff();

    }

}


/*
 * Light Patterns
 *
 */

void LightManager_patternOff(void) {

    LightManager_setSpeed(1);

    *(_manager_instance.output_r) = 0; 
    *(_manager_instance.output_g) = 0; 
    *(_manager_instance.output_b) = 10; 

}

void LightManager_patternSine(void) {

    LightManager_setSpeed(2);

    // calculate theta, in radians, from the current timer
    double theta_rad = ((double)_manager_instance.patternCounter * (double)_manager_instance.patternSpeed/PATTERN_COUNT_TOP) * 2*M_PI;

    // calculate the carrier signal 
    double patternCarrier = sin(theta_rad);

    // update LED intensity
    *(_manager_instance.output_r) = (uint8_t)(30 + (double)(10 * patternCarrier)); 
    *(_manager_instance.output_g) = (uint8_t)(180 + (double)(60 * patternCarrier)); 
    *(_manager_instance.output_b) = (uint8_t)(90 + (double)(30 * patternCarrier)); 

}

void LightManager_patternStrobe(void) {

    LightManager_setSpeed(3);

    // calculate the carrier signal 
    double patternCarrier = (_manager_instance.patternCounter*_manager_instance.patternSpeed > (PATTERN_COUNT_TOP/2)) ? 0 : 1;

    // update LED intensity
    *(_manager_instance.output_r) = (uint8_t)(10 + (30 * patternCarrier)); 
    *(_manager_instance.output_g) = (uint8_t)(60 + (180 * patternCarrier)); 
    *(_manager_instance.output_b) = (uint8_t)(30 + (90 * patternCarrier)); 

}

void LightManager_patternSiren(void) {

    LightManager_setSpeed(10);

    // calculate theta, in radians, from the current timer
    double theta_rad = ((double)_manager_instance.patternCounter * (double)_manager_instance.patternSpeed/PATTERN_COUNT_TOP) * 2*M_PI;

    // calculate the carrier signal 
    double patternCarrier = 0.8 + 0.3 * sin(tan(theta_rad)*0.75);

    // update LED intensity
    *(_manager_instance.output_r) = (uint8_t)(10 + (double)(30 * patternCarrier)); 
    *(_manager_instance.output_g) = (uint8_t)(60 + (double)(180 * patternCarrier)); 
    *(_manager_instance.output_b) = (uint8_t)(30 + (double)(90 * patternCarrier)); 

}

void LightManager_patternSolid(void) {

    LightManager_setSpeed(1);

    // update LED intensity
    *(_manager_instance.output_r) = (uint8_t)(30); 
    *(_manager_instance.output_g) = (uint8_t)(180); 
    *(_manager_instance.output_b) = (uint8_t)(90); 

}


