/**
 * API Client for Developer System Monitor
 * Handles REST API communication with Python backend
 */

class APIClient {
    constructor(baseURL = '') {
        this.baseURL = baseURL;
        this.timeout = 10000; // 10 seconds
    }

    async request(endpoint, options = {}) {
        const controller = new AbortController();
        const timeoutId = setTimeout(() => controller.abort(), this.timeout);

        try {
            const response = await fetch(`${this.baseURL}${endpoint}`, {
                ...options,
                signal: controller.signal,
                headers: {
                    'Content-Type': 'application/json',
                    ...options.headers
                }
            });

            if (!response.ok) {
                throw new Error(`HTTP ${response.status}: ${response.statusText}`);
            }

            return await response.json();
        } catch (error) {
            if (error.name === 'AbortError') {
                throw new Error('Request timeout');
            }
            throw error;
        } finally {
            clearTimeout(timeoutId);
        }
    }

    get(endpoint) {
        return this.request(endpoint);
    }

    post(endpoint, data = {}) {
        return this.request(endpoint, {
            method: 'POST',
            body: JSON.stringify(data)
        });
    }

    put(endpoint, data = {}) {
        return this.request(endpoint, {
            method: 'PUT',
            body: JSON.stringify(data)
        });
    }

    delete(endpoint) {
        return this.request(endpoint, {
            method: 'DELETE'
        });
    }

    // System endpoints
    async getSystemInfo() {
        return this.get('/api/system');
    }

    async getSystemSummary() {
        return this.get('/api/system/summary');
    }

    async getCPUInfo() {
        return this.get('/api/cpu');
    }

    async getMemoryInfo() {
        return this.get('/api/memory');
    }

    async getGPUInfo() {
        return this.get('/api/gpu');
    }

    async getHardwareInfo() {
        return this.get('/api/hardware');
    }

    async getCurrentMetrics() {
        return this.get('/api/metrics/current');
    }

    // Process endpoints
    async getProcesses(params = {}) {
        const queryString = new URLSearchParams(params).toString();
        return this.get(`/api/processes${queryString ? '?' + queryString : ''}`);
    }

    async getProcess(pid) {
        return this.get(`/api/processes/${pid}`);
    }

    async terminateProcess(pid) {
        return this.post(`/api/processes/${pid}/terminate`);
    }

    async suspendProcess(pid) {
        return this.post(`/api/processes/${pid}/suspend`);
    }

    async resumeProcess(pid) {
        return this.post(`/api/processes/${pid}/resume`);
    }

    async getProcessStats() {
        return this.get('/api/processes/stats/summary');
    }

    // Disk endpoints
    async getDisks() {
        return this.get('/api/disks');
    }

    async getDiskUsage() {
        return this.get('/api/disks/usage');
    }

    // Network endpoints
    async getNetworkAdapters() {
        return this.get('/api/network');
    }

    async getNetworkStats() {
        return this.get('/api/network/stats');
    }

    // Services endpoints
    async getServices() {
        return this.get('/api/services');
    }

    async startService(serviceName) {
        return this.post(`/api/services/${serviceName}/start`);
    }

    async stopService(serviceName) {
        return this.post(`/api/services/${serviceName}/stop`);
    }

    async restartService(serviceName) {
        return this.post(`/api/services/${serviceName}/restart`);
    }

    // Settings endpoints
    async getSettings() {
        return this.get('/api/settings');
    }

    async updateSettings(updates) {
        return this.put('/api/settings', { updates });
    }

    async resetSettings() {
        return this.post('/api/settings/reset');
    }

    // Logs endpoints
    async getLogs() {
        return this.get('/api/logs');
    }

    async clearLogs() {
        return this.delete('/api/logs');
    }
}

const api = new APIClient();

if (typeof module !== 'undefined' && module.exports) {
    module.exports = { APIClient, api };
}