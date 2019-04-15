#!/usr/bin/python

from threading import Thread
from time import sleep

class MyThread(Thread):
  def __init__(self, name):
    super().__init__()
    self.name = name
    
  def run(self):
    for i in range(10):
      print("I am thread" , self.name + ":", i)
      sleep(.1)
      
fred = MyThread('Fred')
jane = MyThread('Jane')

fred.start()
jane.start()
