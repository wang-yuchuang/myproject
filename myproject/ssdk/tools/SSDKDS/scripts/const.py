# -*- coding: utf-8 -*-

class _Const(object):
    class ConstError(TypeError):
        pass

    def __setattr__(self, key, value):
        if key in self.__dict__.keys():
            raise self.ConstError("%s is constant, cannot be ressignment !" % key)
        self.__dict__[key] = value

    def __getattr__(self, key):
        if key in self.__dict__.keys():
            return self.key
        else:
            raise self.ConstError("%s not exit !" % key)


class Const(_Const):
    """

    """
