; comparisons
(define <= (lambda (x y)
    (if (< x y)
      1
      (if (eq? x y) 1 ()))))
(define > (lambda (x y) (< y x)))
(define >= (lambda (x y) (<= y x)))

; boolean funs
(define and (lambda (x y)
              (if x 
                (if y 1 ())
                ())))
(define or (lambda (x y) (if x 1 y)))
(define nand (lambda (x y) (null? (and x y))))
