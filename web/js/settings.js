/**
 * Settings Manager Component
 */

class SettingsManager {
    static settings = {};
    static initialized = false;

    static initialize() {
        if (this.initialized) return;
        this.setupEventListeners();
        this.initialized = true;
    }

    static setupEventListeners() {
        // Listen for save button click (delegated)
        document.addEventListener('click', (event) => {
            if (event.target.closest('#save-settings')) {
                this.saveSettings();
            }
            if (event.target.closest('#reset-settings')) {
                this.resetSettings();
            }
        });
    }

    static async load() {
        this.initialize();

        try {
            const settings = await api.getSettings();
            this.settings = settings || {};
            this.renderSettings(this.settings);
        } catch (error) {
            console.error('Failed to load settings:', error);
        }
    }

    static renderSettings(settings) {
        const container = document.getElementById('settings-form');
        if (!container) return;

        const monitoring = settings.monitoring || {};
        const alerts = settings.alerts || {};
        const ui = settings.ui || {};

        container.innerHTML = `
            <!-- Monitoring Settings -->
            <div class="settings-section">
                <div class="settings-section-header">
                    <i class="fas fa-chart-line"></i>
                    <h3>Monitoring</h3>
                </div>
                
                <div class="form-row">
                    <div class="form-group">
                        <label for="monitoring-interval">Update Interval (seconds)</label>
                        <input type="number" id="monitoring-interval" 
                               value="${monitoring.interval_seconds || 2}" 
                               min="1" max="60">
                        <p class="settings-note">How often to refresh system metrics</p>
                    </div>
                    
                    <div class="form-group">
                        <label for="history-retention">History Retention (hours)</label>
                        <input type="number" id="history-retention" 
                               value="${monitoring.history_retention_hours || 24}" 
                               min="1" max="720">
                        <p class="settings-note">How long to keep performance history</p>
                    </div>
                </div>
                
                <div class="form-row">
                    <div class="form-group">
                        <label>GPU Monitoring</label>
                        <label class="toggle-switch">
                            <input type="checkbox" id="enable-gpu" 
                                   ${monitoring.enable_gpu_monitoring ? 'checked' : ''}>
                            <span class="toggle-slider"></span>
                        </label>
                    </div>
                    
                    <div class="form-group">
                        <label>Temperature Monitoring</label>
                        <label class="toggle-switch">
                            <input type="checkbox" id="enable-temperature" 
                                   ${monitoring.enable_temperature_monitoring ? 'checked' : ''}>
                            <span class="toggle-slider"></span>
                        </label>
                    </div>
                </div>
            </div>
            
            <!-- Alert Settings -->
            <div class="settings-section">
                <div class="settings-section-header">
                    <i class="fas fa-bell"></i>
                    <h3>Alerts</h3>
                </div>
                
                <div class="form-group">
                    <label>Enable Alerts</label>
                    <label class="toggle-switch">
                        <input type="checkbox" id="enable-alerts" 
                               ${alerts.enable_alerts ? 'checked' : ''}>
                        <span class="toggle-slider"></span>
                    </label>
                </div>
                
                <div class="form-row">
                    <div class="form-group">
                        <label>CPU Threshold (%)</label>
                        <input type="range" id="cpu-threshold" 
                               value="${alerts.cpu_threshold_percent || 90}" 
                               min="1" max="100" 
                               oninput="document.getElementById('cpu-threshold-value').textContent = this.value + '%'">
                        <span class="threshold-value" id="cpu-threshold-value">${alerts.cpu_threshold_percent || 90}%</span>
                    </div>
                    
                    <div class="form-group">
                        <label>Memory Threshold (%)</label>
                        <input type="range" id="memory-threshold" 
                               value="${alerts.memory_threshold_percent || 90}" 
                               min="1" max="100"
                               oninput="document.getElementById('memory-threshold-value').textContent = this.value + '%'">
                        <span class="threshold-value" id="memory-threshold-value">${alerts.memory_threshold_percent || 90}%</span>
                    </div>
                    
                    <div class="form-group">
                        <label>GPU Threshold (%)</label>
                        <input type="range" id="gpu-threshold" 
                               value="${alerts.gpu_threshold_percent || 95}" 
                               min="1" max="100"
                               oninput="document.getElementById('gpu-threshold-value').textContent = this.value + '%'">
                        <span class="threshold-value" id="gpu-threshold-value">${alerts.gpu_threshold_percent || 95}%</span>
                    </div>
                </div>
            </div>
            
            <!-- UI Settings -->
            <div class="settings-section">
                <div class="settings-section-header">
                    <i class="fas fa-palette"></i>
                    <h3>User Interface</h3>
                </div>
                
                <div class="form-row">
                    <div class="form-group">
                        <label for="ui-theme">Theme</label>
                        <select id="ui-theme">
                            <option value="dark" ${ui.theme === 'dark' ? 'selected' : ''}>Dark</option>
                            <option value="light" ${ui.theme === 'light' ? 'selected' : ''}>Light</option>
                        </select>
                    </div>
                    
                    <div class="form-group">
                        <label for="ui-language">Language</label>
                        <select id="ui-language">
                            <option value="en" ${ui.language === 'en' ? 'selected' : ''}>English</option>
                            <option value="ar" ${ui.language === 'ar' ? 'selected' : ''}>العربية</option>
                        </select>
                    </div>
                </div>
            </div>
            
            <!-- Settings Actions -->
            <div class="settings-actions">
                <button class="btn btn-primary" id="save-settings">
                    <i class="fas fa-save"></i> Save Settings
                </button>
                <button class="btn btn-danger" id="reset-settings">
                    <i class="fas fa-undo"></i> Reset to Defaults
                </button>
            </div>
        `;
    }

    static async saveSettings() {
        const updates = {
            monitoring: {
                interval_seconds: parseInt(document.getElementById('monitoring-interval')?.value || 2),
                history_retention_hours: parseInt(document.getElementById('history-retention')?.value || 24),
                enable_gpu_monitoring: document.getElementById('enable-gpu')?.checked || false,
                enable_temperature_monitoring: document.getElementById('enable-temperature')?.checked || false,
            },
            alerts: {
                enable_alerts: document.getElementById('enable-alerts')?.checked || false,
                cpu_threshold_percent: parseInt(document.getElementById('cpu-threshold')?.value || 90),
                memory_threshold_percent: parseInt(document.getElementById('memory-threshold')?.value || 90),
                gpu_threshold_percent: parseInt(document.getElementById('gpu-threshold')?.value || 95),
            },
            ui: {
                theme: document.getElementById('ui-theme')?.value || 'dark',
                language: document.getElementById('ui-language')?.value || 'en',
            }
        };

        try {
            const result = await api.updateSettings(updates);
            app.showNotification('Settings saved successfully', 'success');

            // Apply theme and language immediately
            if (updates.ui.theme) {
                app.applyTheme(updates.ui.theme);
            }
            if (updates.ui.language) {
                app.applyLanguage(updates.ui.language);
            }
        } catch (error) {
            console.error('Failed to save settings:', error);
            app.showNotification('Failed to save settings', 'error');
        }
    }

    static async resetSettings() {
        const confirmed = await app.showConfirmation(
            'Are you sure you want to reset all settings to defaults?',
            'Reset Settings'
        );

        if (!confirmed) return;

        try {
            const result = await api.resetSettings();
            this.settings = result.settings || {};
            this.renderSettings(this.settings);
            app.showNotification('Settings reset to defaults', 'success');
        } catch (error) {
            console.error('Failed to reset settings:', error);
            app.showNotification('Failed to reset settings', 'error');
        }
    }
}

if (typeof module !== 'undefined' && module.exports) {
    module.exports = { SettingsManager };
}