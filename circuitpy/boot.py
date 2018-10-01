import board
import busio
i2c = busio.I2C(board.SCL, board.SDA)
while not i2c.try_lock():
    pass
result = bytearray(32)
i2c.writeto(83, bytes([0x00, 0x00]), stop=False)
i2c.readfrom_into(83, result)
print(result)
