; metacircular evaluator in your Lisp
(define global-env '())

; helper to extend environment
(define (extend-env vars vals base)
  (cons (cons vars vals) base))

; lookup variable
(define (lookup var env)
  (cond
    ((null? env) (print "symbol " var " not found") ())
    ((eq? var (car (car env))) (cadr (car env)))
    (else (lookup var (cdr env)))))

; eval sequence (for lambda bodies)
(define (eval-sequence exps env)
  (if (null? (cdr exps))
      (eval (car exps) env)
      (begin (eval (car exps) env)
             (eval-sequence (cdr exps) env))))

; eval
(define (eval exp env)
  (cond
    ((number? exp) exp)
    ((string? exp) exp)
    ((symbol? exp) (lookup exp env))
    ((pair? exp)
     (let ((head (car exp)) (tail (cdr exp)))
       (cond
         ((eq? head 'quote) (car tail))
         ((eq? head 'if)
          (let ((test (car tail))
                (then (car (cdr tail)))
                (else (car (cdr (cdr tail)))))
            (if (eval test env)
                (eval then env)
                (eval else env))))
         ((eq? head 'define)
          (let ((var (car tail))
                (val (eval (car (cdr tail)) env)))
            (set! global-env (extend-env (list var) (list val) global-env))
            1)) ; return 1 for success
         ((eq? head 'lambda)
          (list 'closure (car tail) (cdr tail) env))
         (else ; procedure application
          (let ((proc (eval head env))
                (args (map (lambda (x) (eval x env)) tail)))
            (apply proc args))))))
    (else (print "Unknown expression type") ()))

)

; apply
(define (apply proc args)
  (cond
    ((procedure? proc) (apply proc args)) ; builtin
    ((and (pair? proc) (eq? (car proc) 'closure))
     (let ((params (cadr proc))
           (body (caddr proc))
           (env (cadddr proc)))
       (eval-sequence body (extend-env params args env))))
    (else (print "Unknown procedure type") ()))
)
