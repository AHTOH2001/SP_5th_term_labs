import random
from config import min_number, max_number, amount, data_file_name


with open(data_file_name, 'w') as fp:
    fp.write(' '.join((str(random.randint(min_number, max_number))
             for _ in range(amount))))
