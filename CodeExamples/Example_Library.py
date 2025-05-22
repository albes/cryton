cat BookCatalog(book author Var):
    obj:
        book author Var
    hom:
        book  -> author

book     = 1
author   = 1001
i = 0
count = 0

while ( i < 100 ):
    if i == 0:
        catalog = BookCatalog(book author (-1))
    else:
        catalog = BookCatalog(book author catalog)


    count = count + 1
    if count == 5:
        author = author + 1
        count = 0

    book = book + 1
    i = i + 1


i = 0
book_count = 0
while ( i < 1000 ):
    if i in catalog:
        book_count = book_count + 1

    i = i + 1

print(book_count)

book = 0
author = 1003

while (book < 1000):
    if (book -> author in catalog):
        print book
    book = book + 1

cat BorrowRecord(borrow_id member book borrow_date BookCatalog):
    obj:
        borrow_id member book
        borrow_date
        BookCatalog
    hom:
        borrow_id -> book
        borrow_id -> member
        borrow_id -> borrow_date

borrow_id   = 5555555
member = 1000000000
borrow_date = 1955
book = 0

i = 0
count = 0
while ( book < 100 ):
    borrow_id = borrow_id + 1
    member = member + 1
    book = book + 1

    if book == 1:
        borrowed_books = BorrowRecord(borrow_id member book borrow_date catalog)
    else:
        borrowed_books = BorrowRecord(borrow_id member book borrow_date borrowed_books)

    count = count + 1
    if count == 5:
        borrow_date = borrow_date + 1
        count = 0


borrow_date   = 1900
while ( borrow_date < 2025 ):
    borrow_date = borrow_date + 1
    if borrow_date in borrowed_books:
        print(borrow_date)

borrow_id   = 5555570
book = 0

while (book < 1000):

    if borrow_id -> book in borrowed_books and book in catalog:
        author = 1000
        while (author < 2001):

            if book -> author in catalog:
                print(book)
                print(author)

            author = author + 1
    
    book = book + 1