import random
import string
import pyperclip

random_string = ''.join(random.choices(string.ascii_letters + string.digits, k=20000))
pyperclip.copy(random_string)
print("Generated string copied to clipboard!")
