=============================================================================
Numpy language notes
=============================================================================
Last updated on 10/4/16; 10/5/16
=============================================================================

*.  In order to obtain dimensions for a numpy matrix, call .shape

correct_logprobs = -np.log(probs[range(num_examples), y])
print correct_logprobs
print correct_logprobs.shape

*.  numpy.zeros_like(a, dtype=None, order='K', subok=True)

    Return an array of zeros with the same shape and type as a given array.

>>> import numpy as np
>>> x = np.arange(6)
>>> print x
[0 1 2 3 4 5]
>>> x = x.reshape((2,3))
>>> print x
[[0 1 2]
 [3 4 5]]
>>> y = np.zeros_like(x)
>>> print y
[[0 0 0]
 [0 0 0]]

*.  numpy.random.randn(d0, d1, ..., dn)

    Return a sample (or samples) from the “standard normal” distribution.

    If positive, int_like or int-convertible arguments are provided, randn
generates an array of shape (d0, d1, ..., dn), filled with random floats
sampled from a univariate “normal” (Gaussian) distribution of mean 0 and
variance 1 (if any of the d_i are floats, they are first converted to
integers by truncation). A single float randomly sampled from the
distribution is returned if no argument is provided.

*.   a = np.array([1,2,3])
a =  [1 2 3]

a2 = np.array( [ [1], [2], [3] ] )

a2 =  
[[1]
 [2]
 [3]]
