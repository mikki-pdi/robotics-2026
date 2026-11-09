import spidev
import RPi.GPIO as GPIO

global spi

PISPI_BUFFER_SIZE = 256 # Buffer size must match between Pi and STM32

# Pin number for our CS pin repurposed as a ready pin
ReadyPin = 8

# Initialise the SPI communications
# You must pass in the pigpio handle
# Speed is optional, defaults to 1MHz
def YEEPiInit(speed=1000000):
    
    # Global these so they can be used later
    global spi
    
    spi = spidev.SpiDev() # Create SPI object
    spi.open(0,0) # Open SPI bus 0 device 0
    spi.max_speed_hz = speed # Set speed to inputted speed
    spi.mode = 0 # Set SPI mode to 0
    spi.no_cs = True # Disable chip select
    
    GPIO.setmode(GPIO.BCM)
    GPIO.setup(ReadyPin, GPIO.IN, pull_up_down=GPIO.PUD_OFF)


# Execute a transfer to YEEP, exchanging buffers
# Takes a bytearray of values and returns a bytearray
# with the received data
#
# Input data smaller than the buffer size will be padded to correct size
# with null bytes. Input data larger will be truncated
# Warnings will be printed in the cases of an array being truncated
# or if received data is blank
  
def YEEPiTransfer(TXBuf):
    # Check that the input data is a byte array, error if not
    if not isinstance(TXBuf, bytearray):
        raise TypeError("Data must be byte array")
    
    # Check if the array is larger, and delete extra entries
    if len(TXBuf) > PISPI_BUFFER_SIZE:
        print("Warning: Array larger than buffer size, truncating")
        del TXBuf[PISPI_BUFFER_SIZE:] # Delete entries beyond buffer size
    
    # Pad the array to the full size if required
    TXBuf = TXBuf.ljust(PISPI_BUFFER_SIZE, b'\0') #Pad to buffer size with null bytes
    
    # Wait until ready pin is asserted
    while not GPIO.input(ReadyPin):
        pass # Pass immediatly restarts the loop with no delay - we don't want to wait any longer than we have to
    
    # Execute the transfer
    RXBuf = spi.xfer(TXBuf)
    
    # If the received buffer is all 0, warn the user
    if sum(RXBuf) == 0:
        print("Warning: Empty message received")
    
    # Convert returned list to bytearray
    return bytearray(RXBuf)
    
# This should be called when the program exits    
def YEEPiExit():
    GPIO.cleanup()