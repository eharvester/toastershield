// SMD Reflow Definitions
// change the SMD reflow parameters here
//
/*
                     ReflowTempMax  -
                                   / \
                                  /   \
                                 /     \
                                /       \
 PreheatTemp -------------------         \ 
 SoakTemp  -/-                 :          \
           /:                  :           \
          / :                  :            \
        -/- :                  :            -\---ReadyTemp
        /   :                  :              \
       /    :                  :               \
 ------     :                  :                ---
            |<--- SoakTime --->|              
*/
// values for 35-65 Sn-Pb solder 
double PreheatTemp = 150.0;  // temp in C 
double SoakTemp = 140.0; // temp in C, when soak time starts
unsigned long SoakTime = 90000; // soak time in ms
double ReflowTempMax = 205.0; // maximum temp, swings over quickly!
double ReadyTemp = 50.0;  // temp in C, when oven becomes ready for next cycle 
double FanHysteresis = 2.0; // temp in C, fan is turned on/off at ReadyTemp
