/**
 * Measure time it takes read and write to the FPGA. 
 *
 * The purpose of this profiling script is to better understand how many sensors
 * and actuators a Device Node can support. For reads, this script measures how 
 * long the API call took. For writes, this script measures time it takes to
 * write the value and also time it takes for a written value to be reflected in 
 * a read. 
 *
 *                       ---- HARDWARE SETUP ---- 
 *
 * To run this script, AO0-AO3 must be connected to AI0-AI3. This allows the 
 * script to measure time it takes to set an analog output and for that value 
 * to be reflected at an analog in pin. Digital I/O pins do not need any 
 * hardware setup.
 *
 */

# ifndef PROFILE_FPGA_API_HPP
# define PROFILE_FPGA_API_HPP

namespace ProfileFpgaApi
{
    void main (int, char**);
}

# endif
