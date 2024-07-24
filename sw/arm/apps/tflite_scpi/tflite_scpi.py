from pynq import x_heep
import os

# Estendi la classe x_heep per aggiungere debug
class x_heep_debug(x_heep):

    def compile_app(self, app_name):
        print(f"Compiling application: {app_name}")
        # Aggiungi ulteriori stampe di debug
        super().compile_app(app_name)
        print("Compilation complete")

    def run_app(self):
        print("Running application")
        # Aggiungi ulteriori stampe di debug
        super().run_app()
        print("Application execution complete")


os.chdir("home/g.monteasi/x-heep-femu-tflite-sdk/")
# Load the X-HEEP bitstream
x_heep_dbg = x_heep_debug()

# Compile the application
x_heep_dbg.compile_app("tflite_scpi")

# Run the application
x_heep_dbg.run_app()