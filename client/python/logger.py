import logging

console = logging.StreamHandler()
format_str = "%(asctime)s\t%(levelname)s -- %(filename)s:%(funcName)s:%(lineno)s -- %(message)s"
console.setFormatter(logging.Formatter(format_str))

log = logging.getLogger(__name__)
log.addHandler(console)
log.setLevel(logging.INFO)
