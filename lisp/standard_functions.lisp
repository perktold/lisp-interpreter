;; comparisons
(define <= (lambda (x y)
    (if (< x y)
      1
      (if (equal? x y) 1 ()))))
(define > (lambda (x y) (< y x)))
(define >= (lambda (x y) (<= y x)))

(define even? (lambda (n)
               (if (equal? n 0)
                 1
                 (odd? (- n 1)))))

(define odd? (lambda (n)
               (if (equal? n 0)
                 ()
                 (even? (- n 1)))))
;; boolean funs
(define and (lambda (x y)
              (if x 
                (if y 1 ())
                ())))
(define or (lambda (x y) (if x 1 y)))
(define xor (lambda (x y) (if x (null? y) y)))
(define nand (lambda (x y) (null? (and x y))))

;; common recursive functions
(define ! (lambda (n)
            (if (equal? n 0)
              1
              (* n
                 (! (- n 1))))))

(define fib (lambda (n)
              (if (<= n 2)
                1
                (+ (fib (- n 1))
                   (fib (- n 2))))))

(define ack (lambda (n m)
              (if (equal? n 0)
                (+ m 1)
                (if (equal? m 0)
                  (print (ack (print (- n 1)) (print 1)))
                  (ack (- n 1) (ack n (- m 1)))))))

(define seq (lambda (n)
              (seq (print (+ n 1)))))

