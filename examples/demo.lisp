; Recursive factorial
(define (factorial n)
  (if (= n 0)
      1
      (* n (factorial (- n 1)))))

(display (factorial 10))
(newline)

; Fibonacci
(define (fib n)
  (if (< n 2)
      n
      (+ (fib (- n 1)) (fib (- n 2)))))

(display (fib 15))
(newline)

; Tail-recursive loop -- demonstrates TCO, this would stack-overflow without it
(define (count-up n max)
  (if (> n max)
      'done
      (begin
        (display n) (display " ")
        (count-up (+ n 1) max))))

(count-up 1 10)
(newline)

; Higher-order functions: map implemented in the language itself
(define (my-map f lst)
  (if (null? lst)
      '()
      (cons (f (car lst)) (my-map f (cdr lst)))))

(define (square x) (* x x))
(display (my-map square (list 1 2 3 4 5)))
(newline)

; Closures
(define (make-adder n)
  (lambda (x) (+ x n)))

(define add5 (make-adder 5))
(display (add5 10))
(newline)
