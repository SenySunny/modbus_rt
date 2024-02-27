import _aht20


class AHT20(_aht20.Aht20):
    def read(self):
        return [self.readTemp(), self.readHumidity()]
