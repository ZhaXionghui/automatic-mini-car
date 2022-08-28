""" Full assembly of the parts to form the complete network """

from .unet_parts import *

n = 4

class UNet(nn.Module):
    def __init__(self, n_channels, n_classes, bilinear=False):
        super(UNet, self).__init__()
        self.n_channels = n_channels
        self.n_classes = n_classes
        self.bilinear = bilinear

        self.inc = DoubleConv(n_channels, 64 // n)
        self.down1 = Down(64 // n, 128 // n)
        self.down2 = Down(128 // n, 256 // n)
        self.down3 = Down(256 // n, 512 // n)
        factor = 2 if bilinear else 1
        self.down4 = Down(512 // n, 1024 // factor // n)
        self.up1 = Up(1024 // n, 512 // factor // n, bilinear)
        self.up2 = Up(512 // n, 256 // factor // n, bilinear)
        self.up3 = Up(256 // n, 128 // factor // n, bilinear)
        self.up4 = Up(128 // n, 64 // n, bilinear)
        self.outc = OutConv(64 // n, n_classes)

    def forward(self, x):
        x1 = self.inc(x)
        x2 = self.down1(x1)
        x3 = self.down2(x2)
        x4 = self.down3(x3)
        x5 = self.down4(x4)
        x = self.up1(x5, x4)
        x = self.up2(x, x3)
        x = self.up3(x, x2)
        x = self.up4(x, x1)
        logits = self.outc(x)
        return logits
