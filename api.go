package xtray

// Option configures a Tray.
type Option func(*cfg)

// Tray blocks on the macOS run-loop and returns when the application is exited.
// It is safe to call only once.
func Tray(opts ...Option) error {
	c := cfg{
		tooltip:  "",
		iconPath: "",
	}
	for _, op := range opts {
		op(&c)
	}
	return runCore(c)
}

func WithTooltip(s string) Option        { return func(c *cfg) { c.tooltip = s } }
func WithIcon(path string) Option        { return func(c *cfg) { c.iconPath = path } }
func WithLaunchCallback(f func()) Option { return func(c *cfg) { c.launch = f } }
func WithQuitCallback(f func()) Option   { return func(c *cfg) { c.quit = f } }
