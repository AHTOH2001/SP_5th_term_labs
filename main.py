from proccessings import smart, naive

if __name__ == '__main__':
    res1 = naive()
    print()
    res2 = smart()
    print('Program ended')
    if res1 == res2:
        print('Results are equal')
    else:
        print('Results are NOT equal')
