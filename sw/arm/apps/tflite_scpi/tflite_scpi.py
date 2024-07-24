from sdk import x_heep_api 
from x_heep_api import x_heep
import os


# Load the X-HEEP bitstream
x_heep_dbg = x_heep()

# Compile the application
print("Compiling application: tflite_scpi")
x_heep_dbg.compile_app("tflite_scpi")
print("Compilation complete")

# Run the application
print("Running application")
x_heep_dbg.run_app()




"""
print("changing directory")
os.chdir("/home/g.monteasi/x-heep-femu-tflite-sdk/")
print("directory changed")"""