from math import *
import numpy as np
import pygame

SCREEN_WIDTH = 320
SCREEN_HEIGHT = 240

def color565(r, g, b):
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)


class IMUData:
    def __init__(self):
        self.accel_x = 0.0
        self.accel_y = 0.0
        self.accel_z = 1.0


class FakeIMU:
    """Placeholder for BMI270. Arrow keys fill in accel_x / accel_y."""
    def __init__(self):
        self.data = IMUData()

    def get_sensor_data(self):
        keys = pygame.key.get_pressed()
        self.data.accel_x = float(keys[pygame.K_RIGHT] - keys[pygame.K_LEFT])
        self.data.accel_y = float(keys[pygame.K_DOWN] - keys[pygame.K_UP])


imu = FakeIMU()

class Particle:
    def __init__(self, x, y, prevX, prevY):
        self.x = x
        self.y = y
        self.prevX = prevX
        self.prevY = prevY
        self.accX = 0
        self.accY = 0

    def update(self, dt):
        temp_posX = self.x
        self.x = self.x * 2 - self.prevX + self.accX * dt * dt
        self.prevX = temp_posX

        temp_posY = self.y
        self.y = self.y * 2 - self.prevY + self.accY * dt * dt
        self.prevY = temp_posY


pbuffer = []


#def push_pixels(screen):
#    """~ tft.pushPixelsDMA(buffer, W*H)"""
#    img = buffer.reshape(SCREEN_HEIGHT, SCREEN_WIDTH)
#    r = ((img >> 11) & 0x1F) * 255 // 31
#    g = ((img >> 5) & 0x3F) * 255 // 63
#    b = (img & 0x1F) * 255 // 31
#    arr = np.stack([r, g, b], axis=-1).astype(np.uint8)
#    surf = pygame.surfarray.make_surface(arr.transpose(1, 0, 2))
#    screen.blit(pygame.transform.scale(surf, screen.get_size()), (0, 0))
#   pygame.display.flip()


def main():
    # setup()
    pygame.init()
    screen = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT))

    
    # init simulation

    for i in range(2000):
        pbuffer.append(Particle(10 + 5 * (i % 60), 10 + 5 * (i // 60),10 + 5 * (i % 60), 10 + 5 * (i // 60)))

    # loop()
    running = True
    while running:
        for e in pygame.event.get():
            if e.type == pygame.QUIT:
                running = False

        screen.fill((0, 0, 0))
        
        for p in pbuffer:
            p.accY = imu.data.accel_y * 5
            p.accX = imu.data.accel_x * 5

            p.update(0.005)

            pygame.draw.circle(
                screen,
                (255, 255, 255),
                (int(p.x), int(p.y)),
                2
            )

        pygame.display.flip()
        #pygame.time.delay(5)

    pygame.quit()


if __name__ == "__main__":
    main()