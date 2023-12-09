{
  CH552T = {
    gpios = [
      10, 11, 12, 13, 14, 15, 16, 17,
      30, 31, 32, 33, 34, 35,
    ],
    features = {
      uart_30_31 = {
        type = 'uart,
        keys = ["uart0", "uart00"],
        pins = [30, 31],
      },
      uart_12_13 = {
        type = 'uart,
        keys = ["uart0", "uart01"],
        pins = [12, 13],
      },
    },
  },

  CH552G = {
    gpios = [
          11,         14, 15, 16, 17,
      30, 31, 32, 33, 34,
    ],
    features = {
      uart_30_31 = {
        type = 'uart,
        keys = ["uart0", "uart00"],
        pins = [30, 31],
      },
    },
  },

  CH552E = {
    gpios = [
      14, 15, 16, 17,
    ],
  },
}
