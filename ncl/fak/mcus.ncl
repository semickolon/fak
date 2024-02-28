let rec mcus = {
  CH552T = {
    family = 'CH552,
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
    family = 'CH552,
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
    family = 'CH552,
    gpios = [
      14, 15, 16, 17,
    ],
  },

  CH559L = {
    family = 'CH559,
    gpios = [
      00, 01, 02, 03, 04, 05, 06, 07,
      10, 11, 12, 13, 14, 15, 16, 17,
      20, 21, 22, 23, 24, 25, 26, 27,
      30, 31, 32, 33, 34, 35, 36, 37,
    ],
    features = {
      # TODO: [CH559] UART not yet tested.
      uart_30_31 = {
        type = 'uart,
        keys = ["uart0", "uart00"],
        pins = [30, 31],
      },
      uart_02_03 = {
        type = 'uart,
        keys = ["uart0", "uart01"],
        pins = [02, 03],
      },
    },
  },

  CH558L = CH559L,
}
in

mcus
