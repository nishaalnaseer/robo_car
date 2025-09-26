import json
import logging
from typing import List
import subprocess
from certbot.main import main as certbot_main
import sys

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(message)s",
    datefmt="%Y-%m-%d %H:%M:%S",
)

def main():
    # Example: get a cert using standalone mode for example.com
    logging.info("Starting Certbot run...")
    with open("config.json", "r") as file:
        config: dict = json.load(file)

    domains: List[str] = config["domains"]
    nginx_containers: List[str] = config["nginx_containers"]

    args = [
        "certbot",
        "renew",
        "--webroot", "-w", "/var/lib/docker/volumes/hpvolume/_data/letsencrypt/webroot",
        "--non-interactive",
        "--agree-tos",
        "-m", "support@senahiya.mv",
        "--config-dir", "/var/lib/docker/volumes/hpvolume/_data/letsencrypt/config",
        "--work-dir", "/var/lib/docker/volumes/hpvolume/_data/letsencrypt/work",
        "--logs-dir", "/var/lib/docker/volumes/hpvolume/_data/letsencrypt/log",
    ]
    for domain in domains:
        args.append("-d")
        args.append(domain)

    sys.argv = args
    try:
        out = certbot_main()
        logging.info("Certbot finished with result code: %s", out)
    except Exception as exc:
        logging.error(f"Certbot run failed {exc}")
        return

    for container in nginx_containers:
        command = (f"docker compose exec --user root {container}"
                   f" /usr/sbin/nginx -s reload").split(" ")
        logging.info("Reloading Nginx in container: %s", container)
        docker_exec = subprocess.run(command, capture_output=True, text=True)

        if docker_exec.returncode == 0:
            logging.info("Successfully reloaded Nginx in %s", container)
        else:
            logging.error(
                "Failed to reload Nginx in %s (exit %s): %s",
                container,
                docker_exec.returncode,
                docker_exec.stderr.strip(),
            )

if __name__ == '__main__':
    main()

