(define (arithmetic a b c)
	(+ (* (+ a 2) (+ b 3) (+ c 4)) (/ a b) (modulo (round (/ c b)) 2) (- a b c))
)

(define (main)
	(display (arithmetic 1.0 3.0 5.0))
	(newline)
	(display (arithmetic 1.1 3.3 5.5))
	(newline)
)

(main)
