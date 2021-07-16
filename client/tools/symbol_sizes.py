import fileinput

last_size = 25000
last_item = None
entries = []
sum = 0
head = 0

for line in fileinput.input():
    size_, entry_ = line.split(",")
    entry_ = entry_.strip()
    if entry_.startswith("i_"):
        continue

    if head == 0:
        if entry_ == "start":
            head = size_
        else:
            continue

    size_ = int(size_, 16)
    if size_ > 65535:
        continue

    diff = size_ - last_size

    if diff <= 0:
        last_size = size_
        continue

    if diff >= 64:
        entries.append((last_item, diff))
        sum += diff

    last_size = size_
    last_item = entry_

for key, value in sorted(entries, key=lambda x: x[1], reverse=True):
    print("{0}: {1}".format(key, value))

print("Sum: {0}".format(sum))
